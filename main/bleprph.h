#include "esp_peripheral.h"
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
#define GATT_SVR_SVC_ALERT_UUID 0x1811
#define INVALID_TIMER "invalid_timer"
volatile static time_t current_time = 0;
int8_t check_recived_data(char*);
bool get_state(pin_state*);
void set_state(pin_state*,bool);
int gatt_svr_init(void);