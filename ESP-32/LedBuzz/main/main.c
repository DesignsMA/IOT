#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h" // Funciones GPIO y estructuras


#define BUZZER_PIN GPIO_NUM_17
#define LED_PIN GPIO_NUM_5
#define BUTTON_PIN GPIO_NUM_14

gpio_config_t output_config = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = (1ULL << LED_PIN) | (1ULL << BUZZER_PIN),
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
};

gpio_config_t input_config = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = (1ULL << BUTTON_PIN),
    .pull_down_en = GPIO_PULLDOWN_DISABLE, // No necesario, fisicamente el boton ya tiene una resistencia de pull-down
    .pull_up_en = GPIO_PULLUP_DISABLE,
};

enum Command {
    CMD_EXIT,
    CMD_LED_ON,
    CMD_LED_OFF,
    CMD_BUTTON_PRESSED,
};

#define BUFFER_SIZE 16

TaskHandle_t task_handle = NULL;

void task_core1(void *pvParameters) // tarea paralela ejecutada en el Core 1, espera comandos
{
    uint32_t command;

    while (1)
    {
        // Esperar una orden inicial
        xTaskNotifyWait(
            0, // No limpiar ninguna notificación
            ULONG_MAX, // Limpiar todas las notificaciones
            &command, // Variable donde se almacenará la notificación recibida
            portMAX_DELAY // Esperar indefinidamente hasta recibir una notificación
        );
        process_command:
            gpio_set_level(BUZZER_PIN, 0); // Apagar buzzer
            gpio_set_level(LED_PIN, 0); // Apagar LED
            switch (command)
            {
                case CMD_EXIT:
                    printf("Core 1: Recibí orden de salir, terminando tarea...\n");
                    vTaskDelete(NULL); // Terminar la tarea actual
                    break;

                case CMD_LED_ON:
                    printf("Core 1: Recibí orden ON\n");
                    gpio_set_level(LED_PIN, 1); // Encender LED
                    gpio_set_level(BUZZER_PIN, 1); // Encender buzzer
                    printf("Buzzer por 2 segundos...\n");

                    if (xTaskNotifyWait(
                            0,
                            ULONG_MAX,
                            &command, // sobreescribir command con la nueva notificación si llega
                            pdMS_TO_TICKS(2000) // esperar 2 segundos a menos que llegue otra orden
                        ) == pdTRUE)
                    {
                        printf("Core 1: Recibí otra orden antes de apagar el buzzer\n");
                        goto process_command; // Si llega otra orden, procesarla inmediatamente
                    }
                    gpio_set_level(BUZZER_PIN, 0); // Apagar buzzer
                    gpio_set_level(LED_PIN, 0); // Apagar LED
                    break;
            
                case CMD_LED_OFF:
                    printf("Core 1: Recibí orden OFF\n");
                    gpio_set_level(LED_PIN, 0); // Apagar LED
                    gpio_set_level(BUZZER_PIN, 1); // Encender buzzer
                    printf("Buzzer por 2 segundos...\n");

                    if (xTaskNotifyWait(
                            0,
                            ULONG_MAX,
                            &command, // sobreescribir command con la nueva notificación si llega
                            pdMS_TO_TICKS(2000) // esperar 2 segundos a menos que llegue otra orden
                        ) == pdTRUE)
                    {
                        goto process_command; // Si llega otra orden, procesarla inmediatamente
                    }
                    gpio_set_level(BUZZER_PIN, 0); // Apagar buzzer
                    gpio_set_level(LED_PIN, 0); // Apagar LED
                    break;

                case CMD_BUTTON_PRESSED:
                    printf("Core 1: Recibí orden BUTTON_PRESSED\n");
                    gpio_set_level(LED_PIN, 1); // Apagar LED
                    printf("LED por 15 segundos...\n");

                    if (xTaskNotifyWait(
                            0,
                            ULONG_MAX,
                            &command, // sobreescribir command con la nueva notificación si llega
                            pdMS_TO_TICKS(15000) // esperar 15 segundos a menos que llegue otra orden
                        ) == pdTRUE)
                    {
                        printf("Core 1: Recibí otra orden antes de continuar el Buzzer\n");
                        goto process_command; // Si llega otra orden, procesarla inmediatamente
                    }

                    printf("Buzzer por 3 segundos...\n");
                    gpio_set_level(BUZZER_PIN, 1); // Encender buzzer

                    if (xTaskNotifyWait(
                            0,
                            ULONG_MAX,
                            &command, // sobreescribir command con la nueva notificación si llega
                            pdMS_TO_TICKS(3000) // esperar 3 segundos a menos que llegue otra orden
                        ) == pdTRUE)
                    {
                        printf("Core 1: Recibí otra orden antes de apagar el Buzzer y LED\n");
                        goto process_command; // Si llega otra orden, procesarla inmediatamente
                    }

                    gpio_set_level(BUZZER_PIN, 0); // Apagar buzzer
                    gpio_set_level(LED_PIN, 0); // Apagar LED
                    printf("Buzzer y LED apagados.\n");
                    break;
                default:
                    printf("Core 1: Recibí orden desconocida: %lu\n", command);
                    break;
            }
    }
}



void app_main(void) // Función principal del programa, ejecutada en FreeRTOS
{
    gpio_config(&output_config); // Configura los pines de salida (LED y buzzer)
    gpio_config(&input_config); // Configura el pin de entrada (botón)
    short core_id = xPortGetCoreID();
    char buffer[BUFFER_SIZE];
    char c;
    int pos = 0;

    printf("app_main ejecutándose en Core %d\n", core_id);

    xTaskCreatePinnedToCore(
        task_core1, // Función de la tarea
        "task_core1", // Nombre de la tarea
        2048, // Tamaño de la pila
        NULL, // Parámetro de la tsarea
        1, // Prioridad de la tarea
        &task_handle, // Handle de la tarea
        1 // Core donde se ejecutará la tarea (Core 1)
    );

    while (1)
    {
        if (gpio_get_level(BUTTON_PIN) == 1) // Si el botón está presionado
        {
            printf("Core %d: Botón presionado, enviando notificación a Core 1...\n", core_id);
            xTaskNotify(task_handle, CMD_BUTTON_PRESSED, eSetValueWithOverwrite); // Notificar a la tarea en Core 1
            vTaskDelay(pdMS_TO_TICKS(100)); // Esperar 100 ms para evitar rebotes del botón
        }

        // Leer sin bloquear
        if (read(STDIN_FILENO, &c, 1) > 0) // Leer un carácter desde la entrada estándar (puerto serial) sin bloquear
        {
            // Fin del comando
            if (c == '\n' || c == '\r') // Si se recibe un salto de línea o retorno de carro, se considera el fin del comando
            {
                if (pos > 0) // Si hay caracteres en el buffer, procesar el comando
                {
                    buffer[pos] = '\0';

                    printf("Comando recibido: [%s]\n", buffer);

                    if (strcmp(buffer, "ON") == 0)
                    {
                        xTaskNotify(task_handle, CMD_LED_ON, eSetValueWithOverwrite); // Notificar a la tarea en Core 1
                    }
                    else if (strcmp(buffer, "OFF") == 0)
                    {
                        xTaskNotify(task_handle, CMD_LED_OFF, eSetValueWithOverwrite); // Notificar a la tarea en Core 1
                    }
                    else if (strcmp(buffer, "EXIT") == 0)
                    {
                        xTaskNotify(task_handle, CMD_EXIT, eSetValueWithOverwrite); // Notificar a la tarea en Core 1
                        break; // Salir del bucle principal y terminar la tarea
                    }
                    else if (strcmp(buffer, "BUTTON") == 0)
                    {
                        xTaskNotify(task_handle, CMD_BUTTON_PRESSED, eSetValueWithOverwrite); // Notificar a la tarea en Core 1
                    }
                    else
                    {
                        printf("Comando desconocido\n");
                    }

                    // Preparar siguiente comando
                    pos = 0;
                }
            }
            else
            {
                // Guardar carácter
                if (pos < BUFFER_SIZE - 1)
                {
                    buffer[pos++] = c;
                }
            }

        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
