/* Programa principal para controlar cuatro LEDs, un Buzzer y un receptor IR utilizando el ESP32.
   
   Hecho por: Marco Ciprian
   Revisa la licencia en el archivo LICENSE dentro del repositorio del proyecto.
   https://github.com/DesignsMA/IOT

*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "htcw_rmt_ir.h"

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

// Envia notificacion a tarea de comandos
static void notify_command(uint32_t command)
{
    xTaskNotify(command_task_handle, command,
                eSetValueWithOverwrite);
}

static void set_leds(int* states)
{
    for (size_t i = 0; i < 4; i++)
    {
        gpio_set_level(LED_PIN1 + i, states[i]);
    }
    
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
            set_leds((int[]){0, 0, 0, 0}); // Apagar todos los LEDs

            switch (command) {
                case IR_CODE_1:
                    printf("IR: ON\n");
                    set_leds((int[]){1, 0, 0, 0});
                    gpio_set_level(BUZZER_PIN, 1);

                    if (xTaskNotifyWait(0, ULONG_MAX, &command, pdMS_TO_TICKS(2000)) == pdTRUE) {
                        goto process_command;
                    }
                    break;

                case IR_CODE_EXIT:
                    printf("IR: EXIT\n");
                    vTaskDelete(NULL); // Terminar la tarea
                    break;

                default:
                    break;
            }

            gpio_set_level(BUZZER_PIN, 0);
            set_leds((int[]){0, 0, 0, 0});
    }
}



void app_main(void)
{
    rmt_ir_recv_handle_t receiver;
    rmt_ir_vendor_t brand;
    uint32_t code;
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
        }

        vTaskDelay(pdMS_TO_TICKS(10));

        if (code == IR_CODE_EXIT) {
            break;
        }
    }

    gpio_set_level(BUZZER_PIN, 0);
    set_leds((int[]){0, 0, 0, 0});
    gpio_set_level(IR_RECEIVER_SOURCE, 0); // Apagar la fuente de alimentación del receptor IR
    rmt_ir_recv_del(receiver);
}
