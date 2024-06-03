/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "bleprph.h"
#include <math.h>
#include "services/ans/ble_svc_ans.h"
#define TAG "Personal"
/*** Maximum number of characteristics with the notify flag ***/
#define MAX_NOTIFY 5


static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt,
                void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                .uuid = BLE_UUID16_DECLARE(0xFEF4),
                .access_cb = gatt_svc_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, 
            {0,}
        },
    },
    {0,},
};
int8_t check_recived_data(char* data)
{
    uint8_t nr_relay=0;
    char comparare[7];
    memcpy(comparare,data,7);
    switch(data[0])
    {
        case '0':
        {
                static bool config_time = true;
                if(config_time){
                    if(strcmp(comparare,"0x0000") == 0)
                    {
                        //inti_time(1715126300);
                    }
                    config_time = false;
                }
        }break;
        case '1':
        {
            nr_relay |= 0;
        }break;
        case '2':
        {
            nr_relay |= 1<<6;
        }break;
        case 'D':{
            if(strcmp(data,"DXFFFF") == 0)
            {
                return 99;
            }
            else
            {
                ESP_LOGI(TAG,"Valoare eronata");
                return -1;
            }
        }break;
        default:
        ESP_LOGI(TAG,"Valoare eronata");
        return -1;
    }
    memset(comparare,'\000',7);
    ESP_LOGW(TAG,"ASA %s",comparare);
    memcpy(comparare,&data[2],3);
    comparare[3] = '\000';
    if(strcmp("000",comparare) != 0)
    {
        ESP_LOGI(TAG,"Erorre de 0");
        return -1;
    }
    nr_relay += data[strlen(data)-1] - 47;
    if(nr_relay > 69)
    {
        return -1;
    }
    if(nr_relay<64 && nr_relay>5 && nr_relay==0)
    {
        return -1;
    }
    return nr_relay;
}
void set_state(pin_state* pin,bool state)
{
    if(state)
    {
        gpio_hold_dis(pin->pin);
        gpio_set_level(pin->pin,1);
        pin->state = true;
        gpio_hold_en(pin->pin);
    }
    else
    {
        gpio_hold_dis(pin->pin);
        gpio_set_level(pin->pin,0);
        pin->state = false;
        gpio_hold_en(pin->pin);
    }
}
bool get_state(pin_state* pin)
{
    if(pin->state)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static void periodic_timer_run(void *arg);
void inti_time(pin_state* pin)
{
    time_t interval;
    struct tm* timeinfo = localtime(&current_time);
    ESP_LOGI(TAG,"Ore: %d Minute: %d Sec: %d ",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    time_t current_time_in_seconds = timeinfo->tm_hour*60*60 + timeinfo->tm_min*60+ timeinfo->tm_sec;
    pin->timers.time_begin = current_time_in_seconds + 60;
    esp_timer_create_args_t timer_periodic = {
        .callback = periodic_timer_run,
        .name = "test",
        .arg = pin,
    };
    if(pin->timers.time_begin > current_time_in_seconds)
    {
        ESP_LOGI(TAG,"Sunt aici");
        interval = pin->timers.time_begin - current_time_in_seconds;
    }
    else
    {
        interval = current_time_in_seconds - pin->timers.time_begin;
        ESP_LOGI(TAG,"Test %lld",interval);
        interval = 86400 - interval;
        ESP_LOGI(TAG,"A ramas %lld",interval);
    }
    ESP_LOGI(TAG,"Timul curent secunde: %lld",current_time_in_seconds);
    ESP_LOGI(TAG,"Timpul setat: %lld",pin->timers.time_begin);
    esp_timer_create(&timer_periodic,&pin->timers.timer_handler);
    ESP_LOGI(TAG,"TIMPUL PENTRU SETARE %lld",interval*1000000);
    esp_timer_start_periodic(pin->timers.timer_handler,interval*1000000);
}
static void periodic_timer_run(void *arg)
{
    time_t interval = 86400000000;
    uint64_t current =  round(esp_timer_get_time()/1000000);
    time_t rr = current_time + current;
    struct tm* timeinfo = localtime(&rr);
    ESP_LOGI(TAG,"Ore: %d Minute: %d Sec: %d ",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    pin_state* pin = (pin_state*)arg;
    esp_timer_stop(pin->timers.timer_handler);
    set_state(pin,true);
    esp_timer_create_args_t timer_periodic = {
        .callback = periodic_timer_run,
        .name = "test",
        .arg = pin,
    };
    esp_timer_create(&timer_periodic,&pin->timers.timer_handler);
    esp_timer_start_periodic(pin->timers.timer_handler,interval);
    ESP_LOGI(TAG,"%lld",pin->timers.time_begin);
    ESP_LOGI(TAG,"%lld",interval);
}

static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
     int rc=0;
    static char send[30];
    static uint8_t characters=0;
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
    {
        rc = os_mbuf_append(ctxt->om,send,characters);
        ESP_LOGI(TAG,"%s",send);
        memset(send,'\000',30);
        characters = 0;
        return rc;
    }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        {
            char data[30];
            memset(data,'\000',30);
            ble_hs_mbuf_to_flat(ctxt->om,data,ctxt->om->om_len,NULL);
            switch(check_recived_data(data))
            {
                case 1:
                {
                    if(get_state(&pins[0]))
                    {
                        strcpy(send,"1_ON");
                        characters = 5;
                    }
                    else
                    {
                        strcpy(send,"1_OFF");
                        characters = 6;
                    }
                }break;
                case 2:
                {
                    pins[0].rellay_state = true;
                    set_state(&pins[0],true);
                    ESP_LOGI(TAG,"Sunt aici Turn ON rellay 1");
                }break;
                case 3:
                {
                    pins[0].rellay_state = true;
                    set_state(&pins[0],false);
                    ESP_LOGI(TAG,"Turn OFF rellay 1");
                }break;
                case 4:
                {
                    pins[0].timer_state = true;
                    pins[0].time_begin = 61825;
                    pins[0].time_end   = 62000;
                    ESP_LOGI(TAG,"Turn ON with timer rellay 1");
                }break;
                case 5:
                {
                    pins[0].timer_state = false;
                    ESP_LOGI(TAG,"TURN OFF timer for rellay 1");
                }break;
                case 65:
                {
                    if(get_state(&pins[1]))
                    {
                        strcpy(send,"2_ON");
                        characters = 5;
                    }
                    else
                    {
                        strcpy(send,"3_OFF");
                        characters = 6;
                    }
                }break;
                case 66:
                {
                    pins[1].rellay_state = true;
                    set_state(&pins[1],true);
                    ESP_LOGI(TAG,"Turn ON rellay 2");     
                }break;
                case 67:
                {
                    pins[1].rellay_state = true;
                    set_state(&pins[1],false);
                    ESP_LOGI(TAG,"Turn OFF rellay 2");
                }break;
                case 68:
                {
                    pins[1].timer_state = true;
                    pins[1].time_begin = 61725;
                    pins[1].time_end   = 61925;
                    ESP_LOGI(TAG,"Turn ON with timer rellay 2");
                }break;
                case 69:
                {
                    pins[1].timer_state = false;
                    ESP_LOGI(TAG,"Turn OFF timer for rellay 2");
                }break;
                case 99:
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
                        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_4, adc_raw);
                        #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
                        ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_4, voltage);
                        float dates = ((float)voltage/1000)-0.565;
                        int sendd =  (int)round((dates*100)/0.959);
                        static int old_send = 100;
                        ESP_LOGI(TAG,"%f",dates);
                        ESP_LOGI(TAG,"%d",sendd);
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
                            ESP_LOGW(TAG,"Se atribuie old: %d", sendd);
                        }
                        else
                        {
                            old_send = sendd;
                            ESP_LOGW(TAG,"Se atribuie new: %d",sendd);
                        }
                        itoa(sendd,send,10);
                        ESP_LOGI(TAG,"dates: %f",dates);
                        ESP_LOGI(TAG,"procentaj: %d",sendd);
                        if(sendd>=100)
                        {
                            characters = 3;
                        }
                        if(sendd>=10 && sendd<100)
                        {
                            characters = 2;
                        }
                        if(sendd<10)
                        {
                          characters = 1;  
                        }
                        ESP_LOGI(TAG,"In string: %s",send);
                        ESP_LOGI(TAG,"Nr caractere: %d",characters);
                        adc_cali_delete_scheme_curve_fitting(adc_calibration);
                        #endif
                }break;
                default:
                {
                    ESP_LOGI(TAG,"Error");
                }break;
            }    
            return rc;
        }
        break;
    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        goto unknown;

    default:
        goto unknown;
    }
unknown:
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;
    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;
    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}
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
void timer()
{
    uint64_t current =  round(esp_timer_get_time()/1000000);
    ESP_LOGI(TAG,"Timpul de la inceput: %lld",current);
    time_t rr = current_time + current;
    struct tm* timeinfo = localtime(&rr);
    ESP_LOGI(TAG,"Ore: %d Minute: %d Sec: %d ",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    
}
int
gatt_svr_init(void)
{
    current_time = 1717346750+10800;
    inti_time(&pins[0]);
    esp_timer_handle_t handler;
    esp_timer_create_args_t asp = 
    {
        .callback = timer,
        .name = "abc"
    };
    esp_timer_create(&asp,&handler);
    esp_timer_start_periodic(handler,30000000);
    adc_init();
    int rc;
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_ans_init();
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }
    return 0;
}