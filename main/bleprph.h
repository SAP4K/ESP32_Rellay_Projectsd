/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_BLEPRPH_
#define H_BLEPRPH_

#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "esp_peripheral.h"
#include "driver/gpio.h"
#ifdef __cplusplus
extern "C" {
#endif

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;


typedef struct pin_state
{
 gpio_num_t pin;
 uint32_t state 
} pin_state;
/** GATT server. */
#define GATT_SVR_SVC_ALERT_UUID 0x1811
#define BATTERY_LEBEL    "DXFFFF"
#define RELAY_1_TURN_ON  "1x0001"
#define RELAY_1_TURN_OFF "1x0002"
#define RELAY_2_TURN_ON  "2x0001"
#define RELAY_2_TURN_OFF "2x0002"
int gatt_svr_init(void);
#ifdef __cplusplus
}
#endif

#endif
