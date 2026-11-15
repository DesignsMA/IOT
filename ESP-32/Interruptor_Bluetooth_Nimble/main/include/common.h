/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef COMMON_H
#define COMMON_H

/* Includes */
/* STD APIs */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* ESP APIs */
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

/* FreeRTOS APIs */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* NimBLE stack APIs */
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "driver/gpio.h" 

/* Defines */
#define TAG "ESP32-S3"
#define DEVICE_NAME "ESP32-S3-DES728"
#define STATIC_PASSKEY  672894

#define LED_COLOR_ADVERTISING  0x00ECFF
#define LED_COLOR_CONNECTED    0x00FF00
#define LED_COLOR_DISCONNECTED 0xFF0000
#define LED_COLOR_OFF           0x000000
#define SWITCH_COLOR_ON 0XFFBB00
#define SWITCH_COLOR_OFF 0XFF4300

#define SWITCH_PIN GPIO_NUM_4

#endif // COMMON_H
