#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
void adc_init();
int adc_get_battery();
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc_calibration = NULL;