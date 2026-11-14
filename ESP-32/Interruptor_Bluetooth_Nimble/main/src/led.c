/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "led.h"
#include "common.h"

/* Private variables */
static uint8_t led_state;
static led_strip_handle_t led_strip;

/* Public functions */
uint8_t get_led_state(void) { return led_state; }

#define LED_QUEUE_LEN  10
#define LED_FADE_STEPS 20
#define LED_FADE_MS 15

typedef struct {
    uint8_t r, g, b;
    uint16_t steps;
    uint16_t step_ms;
} led_req_t;

static QueueHandle_t s_led_q = NULL;
static uint32_t g_led_rgb = 0;

static void led_worker_task(void *pv)
{
    led_req_t q;
    for (;;) {
        if (xQueueReceive(s_led_q, &q, portMAX_DELAY) != pdTRUE) continue;

        led_state = 1;

        uint8_t r0 = (g_led_rgb >> 16) & 0xFF;
        uint8_t g0 = (g_led_rgb >>  8) & 0xFF;
        uint8_t b0 = (g_led_rgb      ) & 0xFF;

        for (uint16_t i = 0; i <= q.steps; i++) {
            uint8_t r = r0 + ((int)q.r - r0) * i / q.steps;
            uint8_t g = g0 + ((int)q.g - g0) * i / q.steps;
            uint8_t b = b0 + ((int)q.b - b0) * i / q.steps;

            led_strip_set_pixel(led_strip, 0, r, g, b);
            led_strip_refresh(led_strip);
            if (i < q.steps) vTaskDelay(pdMS_TO_TICKS(q.step_ms));
        }

        g_led_rgb = ((uint32_t)q.r << 16) | ((uint32_t)q.g << 8) | q.b;
        if (g_led_rgb == 0) led_state = 0;
    }
}

void led_init(void)
{
    ESP_LOGI(TAG, "example configured to blink addressable led!");
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_BLINK_GPIO,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    s_led_q = xQueueCreate(LED_QUEUE_LEN, sizeof(led_req_t));
    ESP_ERROR_CHECK(s_led_q != NULL ? ESP_OK : ESP_ERR_NO_MEM);
    ESP_ERROR_CHECK(xTaskCreate(led_worker_task, "led_w", 3072, NULL, 5, NULL)
                        == pdPASS
                    ? ESP_OK
                    : ESP_FAIL);
    led_off();
}

esp_err_t led_on(uint32_t rgb)
{
    if (!s_led_q) return ESP_ERR_INVALID_STATE;

    led_req_t q = {
        .r       = (rgb >> 16) & 0xFF,
        .g       = (rgb >>  8) & 0xFF,
        .b       =  rgb        & 0xFF,
        .steps   = LED_FADE_STEPS,
        .step_ms = LED_FADE_MS,
    };
    return (xQueueSend(s_led_q, &q, 0) == pdTRUE) ? ESP_OK : ESP_ERR_NO_MEM;
}

void led_off(void) {
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    led_state = 0;
}
