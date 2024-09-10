#include "esp_peripheral.h"
#include "driver/gpio.h"
#include "adc.h"
#include "esp_pm.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include <math.h>
#include "services/ans/ble_svc_ans.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "config_time.h"
#include "nvs_memory.h"
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
 uint8_t repeat;
 bool status_repeat;
 bool status_one[2];
}personal_timer;

typedef struct pin_state
{
 gpio_num_t pin;
 uint32_t state;
 bool rellay_state;
 personal_timer timers;
} pin_state;


#define GATT_SVR_SVC_ALERT_UUID 0x1811
 

volatile static pin_state pins[2] = 
{
#ifdef ESP32_C3
{.pin=GPIO_NUM_3, .state=false, .timers.time_begin_repeat=0, .timers.time_end_repeat=0, .timers.repeat=0, .timers.status_repeat = true, .timers.status_one[0] = true, .timers.status_one[1] = true},
{.pin=GPIO_NUM_2, .state=false, .timers.time_begin_repeat=0, .timers.time_end_repeat=0, .timers.repeat=0, .timers.status_repeat = true, .timers.status_one[0] = true, .timers.status_one[1] = true}
#endif
#ifdef ESP32
{.pin=GPIO_NUM_25, .state=false},
{.pin=GPIO_NUM_26, .state=false}
#endif
};
volatile static time_t current_time = 0;
int8_t check_recived_data(char*);
bool get_state(pin_state*);
void set_state(pin_state*,bool);
int gatt_svr_init(void);
void init_time(pin_state*,bool);
static void timer_on_rellay();
static void timer_off_rellay(void* arg);
static void timer_on_rellay_only_one(void *arg);
static void timer_off_rellay_only_one(void* arg);
void prase_data_form_time(pin_state*,char*,bool,uint16_t);
static ble_addr_t addr;