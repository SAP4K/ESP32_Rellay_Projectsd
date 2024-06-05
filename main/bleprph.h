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
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#define ESP32_C3

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

typedef struct personal_timer
{
 esp_timer_handle_t timer_handler_begin_repeat;
 esp_timer_handle_t timer_handler_end_repeat;
 esp_timer_handle_t timer_handler_begin;
 esp_timer_handle_t timer_handler_end;
 time_t time_begin_repeat;
 time_t time_end_repeat;
 time_t time_begin;
 time_t time_end;
 uint8_t repeat
}personal_timer;
/** GATT server. */
typedef struct pin_state
{
 gpio_num_t pin;
 uint32_t state;
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
{.pin=GPIO_NUM_3, .state=false, .timers.time_begin_repeat=0, .timers.time_end_repeat=0, .timers.repeat=0},
{.pin=GPIO_NUM_2, .state=false, .timers.time_begin_repeat=0, .timers.time_end_repeat=0, .timers.repeat=0}
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
void init_time(pin_state*,bool);
static void timer_on_rellay(void *arg);
static void timer_off_rellay(void* arg);
static void timer_on_rellay_only_one(void *arg);
static void timer_off_rellay_only_one(void* arg);
void adc_init();
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc_calibration = NULL;
void prase_data_form_time(pin_state*,char*,bool,uint16_t);
static nvs_handle_t memory_handler;
static bool testam_alata_denumire = false;
static ble_addr_t addr;
void set_false();