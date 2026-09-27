/* Programa principal para controlar cuatro LEDs, un Buzzer y un receptor IR utilizando el ESP32.
   
   Hecho por: Marco Ciprian
   Revisa la licencia en el archivo LICENSE dentro del repositorio del proyecto.
   https://github.com/DesignsMA/IOT

*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "htcw_rmt_ir.h"
#include "esp_random.h"
#include <math.h>

// Definiciones de pines
#define IR_RECEIVER_SOURCE GPIO_NUM_4
#define IR_RECEIVER_PIN GPIO_NUM_17
#define LED_PIN1 GPIO_NUM_5
#define LED_PIN2 GPIO_NUM_6
#define LED_PIN3 GPIO_NUM_7
#define LED_PIN4 GPIO_NUM_15
#define BUZZER_PIN GPIO_NUM_16

// Definiciones de códigos hexadecimales
#define IR_CODE_1   0x00FF0728
#define IR_CODE_2   0x00FF0729
#define IR_CODE_3   0x00FF072A
#define IR_CODE_4   0x00FF072B
#define IR_CODE_5   0x00FF072C
#define IR_CODE_6   0x00FF072D
#define IR_CODE_7   0x00FF072E
#define IR_CODE_8   0x00FF072F
#define IR_CODE_9   0x00FF0730
#define IR_CODE_10  0x00FF0731
#define IR_CODE_EXIT  0x00FF0732

static TaskHandle_t command_task_handle;
static unsigned short exitFlag=0;

// Envia notificacion a tarea de comandos
static void notify_command(uint32_t command)
{
    xTaskNotify(command_task_handle, command,
                eSetValueWithOverwrite);
}

static void set_leds(int* states)
{
    gpio_set_level(LED_PIN1, states[0]);
    gpio_set_level(LED_PIN2, states[1]);
    gpio_set_level(LED_PIN3, states[2]);
    gpio_set_level(LED_PIN4, states[3]);
}

static void command_task(void *parameter)
{   
    uint32_t command;

    (void)parameter;

    while (true) {
        // Esperar indefinidamente un comando
        xTaskNotifyWait(0, ULONG_MAX, &command, portMAX_DELAY);

        process_command:
            gpio_set_level(BUZZER_PIN, 0);

            switch (command) {
                case IR_CODE_1:
                    printf("Prende LED 1\n");
                    set_leds((int[]){1, 0, 0, 0});
                    break;

                case IR_CODE_2:
                    printf("Prende LED 2\n");
                    set_leds((int[]){0, 1, 0, 0});
                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(1000)) == pdTRUE) {
                        goto process_command;
                    }
                    break;

                case IR_CODE_3:
                    printf("Prendiste el LED 3\n");
                    set_leds((int[]){0, 0, 1, 0});
                    break;
                
                case IR_CODE_4:
                    printf("Prende LED 4\n");
                    set_leds((int[]){0, 0, 0, 1});
                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(2000)) == pdTRUE) {
                        goto process_command;
                    }
                    break;
                
                case IR_CODE_5:
                    printf("Se apagó todo\n");
                    set_leds((int[]){0, 0, 0, 0});
                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(200)) == pdTRUE) {
                        goto process_command;
                    }

                    gpio_set_level(BUZZER_PIN, 0);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(100)) == pdTRUE) {
                        goto process_command;
                    }

                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(200)) == pdTRUE) {
                        goto process_command;
                    }
                    break;
                
                case IR_CODE_6:
                    printf("Se prendió todo\n");
                    set_leds((int[]){1, 1, 1, 1});
                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(200)) == pdTRUE) {
                        goto process_command;
                    }

                    gpio_set_level(BUZZER_PIN, 0);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(100)) == pdTRUE) {
                        goto process_command;
                    }

                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(200)) == pdTRUE) {
                        goto process_command;
                    }
                    break;

                case IR_CODE_7:
                    printf("Secuencia de leds\n");
                    for (int i = 0; i < 4; i++) {
                        int states[4] = {0, 0, 0, 0};
                        states[i] = 1;
                        set_leds(states);
                        if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(200)) == pdTRUE) {
                            goto process_command;
                        }
                    }
                    break;

                case IR_CODE_8:
                    printf("Secuencia de corazon\n");

                    // Secuencia de latido: izquierda -> derecha -> izquierda
                    int heart_sequence[] = {0, 1, 2, 3, 2, 1, 0};

                    // Varios latidos
                    for (int beat = 0; beat < 3; beat++) {

                        for (int j = 0; j < 7; j++) {

                            int states[4] = {0, 0, 0, 0};
                            states[heart_sequence[j]] = 1;

                            set_leds(states);

                            // Pitido mientras el LED está encendido
                            gpio_set_level(BUZZER_PIN, 1);

                            if (xTaskNotifyWait(
                                    0,
                                    ULONG_MAX,
                                    &command,
                                    pdMS_TO_TICKS(100)
                                ) == pdTRUE) {

                                goto process_command;
                            }

                            gpio_set_level(BUZZER_PIN, 0);

                            if (xTaskNotifyWait(
                                    0,
                                    ULONG_MAX,
                                    &command,
                                    pdMS_TO_TICKS(100)
                                ) == pdTRUE) {

                                goto process_command;
                            }
                        }

                        // Pausa entre latidos
                        if (xTaskNotifyWait(
                                0,
                                ULONG_MAX,
                                &command,
                                pdMS_TO_TICKS(300)
                            ) == pdTRUE) {

                            goto process_command;
                        }
                    }

                    // Apagar LEDs antes de la espera final
                    int states[4] = {0, 0, 0, 0};
                    set_leds(states);
                    gpio_set_level(BUZZER_PIN, 0);

                    // Pitido constante al finalizar
                    printf("Pitido constante\n");
                    gpio_set_level(BUZZER_PIN, 1);
                    // Esperar 5 segundos
                    if (xTaskNotifyWait(
                            0,
                            ULONG_MAX,
                            &command,
                            pdMS_TO_TICKS(5000)
                        ) == pdTRUE) {

                        goto process_command;
                    }

                    break;
                
                case IR_CODE_9:
                while (true) {
                    printf("Espero un comando cualquiera...\n");
                    xTaskNotifyWait(0, ULONG_MAX, &command, portMAX_DELAY);
                    if (command == IR_CODE_9) {
                        printf("Se recibió el mismo comando, saliendo de la secuencia de tonos\n");
                        break;
                    }
                    printf("Convirtiendo tu comando en un tono\n");
                    uint32_t tone_command = command;
                    command = 0; // Reiniciar el comando para evitar bucles infinitos
                    int states[] = {0,0,0,0};
                    for (int i = 0; i < 32; i++)
                    {
                        gpio_set_level(BUZZER_PIN, 0);
                        if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(50)) == pdTRUE) {
                            goto process_command;
                        }

                        if (i%4 == 0) {
                            states[0]=states[1]=states[2]=states[3]=0;
                        }

                        gpio_set_level(BUZZER_PIN, 0);
                        set_leds(states);
                        bool state = ((tone_command >> (31 - i)) & 1U) != 0;
                        printf("Bit %d: %d\n", i, state);
                        // delay aleatorio entre pitidos
                        uint32_t delay = 5 + (esp_random() % 300); // entre 5 y 305 ms
                        printf("Delay: %lu ms\n", delay);
                        gpio_set_level(BUZZER_PIN, state); // encender o apagar el buzzer según el bit actual
                        states[i%4] = state; // actualizar cuarteto actual
                        set_leds(states);
                        // delay entre actualizaciones
                        if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(delay)) == pdTRUE) {
                            goto process_command;
                        }
                    }
                    gpio_set_level(BUZZER_PIN, 0);
                    set_leds((int[]){0,0,0,0});
                }
                    break;
                
                case IR_CODE_10:
                    printf("Secuencia acelerada de leds con pitido\n");

                    const int steps = 250;
                    const float delay_start = 600.0f;
                    const float delay_end = 0.05f;

                    for (int step = 0; step < steps; step++) {

                        // Posición normalizada: 0.0 -> 1.0
                        float t = (float)step / (steps - 1);

                        // Aceleración exponencial
                        float delay_ms = delay_start *
                                        powf(delay_end / delay_start, t);

                        // LED de izquierda a derecha
                        int i = step % 4;

                        int states[4] = {0, 0, 0, 0};
                        states[i] = 1;
                        set_leds(states);

                        // Encender buzzer
                        gpio_set_level(BUZZER_PIN, 1);

                        // Mantener LED + buzzer durante el delay
                        if (xTaskNotifyWait(
                                0,
                                ULONG_MAX,
                                &command,
                                pdMS_TO_TICKS((int)delay_ms)
                            ) == pdTRUE) {

                            goto process_command;
                        }

                        // Apagar buzzer
                        gpio_set_level(BUZZER_PIN, 0);

                        if (xTaskNotifyWait(
                                0,
                                ULONG_MAX,
                                &command,
                                pdMS_TO_TICKS((int)delay_ms)
                            ) == pdTRUE) {

                            goto process_command;
                        }
                    }

                    break;


                case IR_CODE_EXIT:
                    printf("IR: EXIT\n");
                    exitFlag = 1; // Establecer la bandera de salida
                    vTaskDelete(NULL); // Terminar la tarea
                    break;

                default:
                    break;
            }

            gpio_set_level(BUZZER_PIN, 0);
    }
}



void app_main(void)
{
    rmt_ir_recv_handle_t receiver;
    rmt_ir_vendor_t brand;
    uint32_t code = 0;
    size_t bits;

    // Configuración de pines GPIO para LEDs y Buzzer
    gpio_config_t gpio_configuration = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_PIN1) | (1ULL << LED_PIN2) | (1ULL << LED_PIN3) | (1ULL << LED_PIN4) | (1ULL << BUZZER_PIN) | (1ULL << IR_RECEIVER_SOURCE),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    // Configuración de pin GPIO para el receptor IR
    gpio_config_t gpio_configuration_ir = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << IR_RECEIVER_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };


    gpio_config(&gpio_configuration);
    set_leds((int[]){0, 0, 0, 0}); // Apagar todos los LEDs
    gpio_set_level(BUZZER_PIN, 0);
    gpio_set_level(IR_RECEIVER_SOURCE, 1); // Fuente de alimentación para el receptor IR
    gpio_config(&gpio_configuration_ir); // Configuración del pin del receptor IR como entrada


    // Tarea de comandos en core 1
    xTaskCreatePinnedToCore(command_task, "command_task", 2048, NULL, 1,
                            &command_task_handle, 1); 

    // Crear el receptor IR
    if (!rmt_ir_recv_create(IR_RECEIVER_PIN, NULL, NULL, &receiver)) {
        printf("No se pudo crear el receptor IR\n");
        return;
    }

    while (true) {
        if (rmt_ir_recv_poll(receiver, &brand, &code, &bits)) {
            printf("IR recibido: marca=%d codigo=0x%08lx bits=%u\n",
                   brand, (unsigned long)code, (unsigned)bits);

            notify_command(code);

            if (exitFlag == 1) {
                printf("Se ha solicitado salir del programa\n");
                break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    gpio_set_level(BUZZER_PIN, 0);
    set_leds((int[]){0, 0, 0, 0});
    gpio_set_level(IR_RECEIVER_SOURCE, 0); // Apagar la fuente de alimentación del receptor IR
    rmt_ir_recv_del(receiver);
    vTaskDelete(NULL);
}
