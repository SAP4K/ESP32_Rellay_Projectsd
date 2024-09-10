#include "esp_peripheral.h"
#include "pins.h"
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
 

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;



#define GATT_SVR_SVC_ALERT_UUID 0x1811
 
volatile static time_t current_time = 0;
int8_t check_recived_data(char*);
bool get_state(pin_state*);
void set_state(pin_state*,bool);
int gatt_svr_init(void);
static ble_addr_t addr;