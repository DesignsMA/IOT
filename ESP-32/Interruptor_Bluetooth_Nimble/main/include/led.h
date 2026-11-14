/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef LED_H
#define LED_H

/* Includes */
/* ESP APIs */
#include <stdint.h>
#include "esp_err.h"
#include "led_strip.h"
#include "sdkconfig.h"

/* Defines */
#define BLINK_GPIO CONFIG_BLINK_GPIO

/* Public function declarations */
uint8_t get_led_state(void);
esp_err_t led_on(uint32_t rgb);
void led_off(void);
void led_init(void);

#endif // LED_H
