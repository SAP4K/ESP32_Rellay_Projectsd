/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "esp_peripheral.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ESP32_C3

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

typedef struct personal_timer
{
 esp_timer_handle_t timer_handler;
 time_t time_begin;
 time_t time_end;
}personal_timer;
/** GATT server. */
typedef struct pin_state
{
 gpio_num_t pin;
 uint32_t state;
 timer_t time_begin;
 timer_t time_end;
 bool timer_state;
 bool rellay_state;
 personal_timer timers;
} pin_state;

//------Microcontroler------


//------Comenzile de la aplicatie------
#define GATT_SVR_SVC_ALERT_UUID 0x1811
#define BATTERY_LEBEL    "DXFFFF"
#define RELAY_1_TURN_ON  "1x0001"
#define RELAY_1_TURN_OFF "1x0002"
#define RELAY_2_TURN_ON  "2x0001"
#define RELAY_2_TURN_OFF "2x0002"

//------Pinii de control al releelor------
static pin_state pins[2] = 
{
#ifdef ESP32_C3
{.pin=GPIO_NUM_3, .state=false, .timer_state=false, .time_begin=0, .time_end=0, .rellay_state=false},
{.pin=GPIO_NUM_2, .state=false, .timer_state=false, .time_begin=0, .time_end=0, .rellay_state=false}
#endif
#ifdef ESP32
{.pin=GPIO_NUM_25, .state=false},
{.pin=GPIO_NUM_26, .state=false}
#endif
};
static time_t current_time = 0;
int8_t check_recived_data(char*);
bool get_state(pin_state*);
void set_state(pin_state*,bool);
int gatt_svr_init(void);
void inti_time(pin_state*);
static void periodic_timer_run(void *arg);
void adc_init();
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc_calibration = NULL;