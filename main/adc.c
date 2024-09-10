#include "adc.h"
#include "math.h"
void adc_init()
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config));
}
int adc_get_battery()
{
     int adc_raw;
     int voltage;
     adc_oneshot_read(adc1_handle,ADC_CHANNEL_4,&adc_raw);
     #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
     adc_cali_curve_fitting_config_t cali_conf = 
     {
          .unit_id = ADC_UNIT_1,
          .atten = ADC_ATTEN_DB_12,
          .bitwidth = ADC_BITWIDTH_12
     };
     adc_cali_create_scheme_curve_fitting(&cali_conf,&adc_calibration);
     adc_cali_raw_to_voltage(adc_calibration,adc_raw,&voltage);
     #endif
     ESP_LOGI("ADC", "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_4, adc_raw);
     #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
     ESP_LOGI("ADC", "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_4, voltage);
     float dates = ((float)voltage/1000)-0.565;
     int sendd =  (int)round((dates*100)/0.959);
     static int old_send = 100;
     ESP_LOGI("ADC","%f",dates);
     ESP_LOGI("ADC","%d",sendd);
     if(sendd > 100)
     {
          sendd = 100;
     }
     if(sendd<1)
     {
          sendd = 0;
     }
     if(old_send < sendd)
     {
          sendd = old_send;
          ESP_LOGW("ADC","Se atribuie old: %d", sendd);
     }
     else
     {
          old_send = sendd;
          ESP_LOGW("ADC","Se atribuie new: %d",sendd);
     }
     ESP_LOGI("ADC","dates: %f",dates);
     ESP_LOGI("ADC","procentaj: %d",sendd);
     adc_cali_delete_scheme_curve_fitting(adc_calibration);
     #endif
     return sendd;
}