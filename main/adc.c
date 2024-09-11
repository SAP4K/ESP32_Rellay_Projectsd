#include "adc.h"
#include "math.h"
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc_calibration = NULL;
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
int adc_get_battery(uint8_t* characters,char* send)
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
     #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
     float dates = ((float)voltage/1000)-0.565;
     int sendd =  (int)round((dates*100)/0.959);
     int old_send = 100;
     if(sendd > 100)
          sendd = 100;

     if(sendd<1)
          sendd = 0;

     if(old_send < sendd)
          sendd = old_send;
     else
          old_send = sendd;
     adc_cali_delete_scheme_curve_fitting(adc_calibration);
     #endif
     return sendd;
     itoa(sendd,send,10);
     if(sendd>=100)
     {
          *characters = 3;
     }
     if(sendd>=10 && sendd<100)
     {
          *characters = 2;
     }
     if(sendd<10)
     {
          *characters = 1;  
     }
     return 0;
}