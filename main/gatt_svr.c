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
    memcpy(comparare,&data[2],3);
    comparare[3] = '\000';
    if(strcmp("000",comparare) != 0)
    {
        ESP_LOGI(TAG,"Erorre de 0");
        return -1;
    }
    nr_relay += data[5] - 47;
    ESP_LOGE(TAG,"Data %d",nr_relay);
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
};
void inti_time(pin_state* pin,bool choose_time)
{
    struct tm* timeinfo = localtime(&pin->timers.time_begin_repeat);
    time_t time_in_second_begin;
    time_t time_in_second_end;
    if(choose_time)
    {
        time_in_second_begin = pin->timers.time_begin_repeat - current_time;
        time_in_second_end = pin->timers.time_end_repeat - current_time;
    }
    else
    {
        time_in_second_begin = pin->timers.time_begin - current_time;
        time_in_second_end = pin->timers.time_end - current_time;
    }
    ESP_LOGI(TAG,"Start: %lld", time_in_second_begin);
    ESP_LOGI(TAG,"End: %lld", time_in_second_end);
    ESP_LOGI(TAG," %d.%d.%d Ore: %d Minute: %d Sec: %d ",timeinfo->tm_year,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    esp_timer_create_args_t start_timer = 
    {
        .name = "Start",
        .arg = pin
    };
    esp_timer_create_args_t stop_timer = 
    {
        .name = "Stop",
        .arg = pin
    };
    if(choose_time)
    {
        start_timer.callback = timer_on_rellay;
        esp_timer_create(&start_timer,&pin->timers.timer_handler_begin_repeat);
        esp_timer_start_periodic(pin->timers.timer_handler_begin_repeat,time_in_second_begin*1000000);
        stop_timer.callback = timer_off_rellay;
        esp_timer_create(&stop_timer,&pin->timers.timer_handler_end_repeat);
        esp_timer_start_periodic(pin->timers.timer_handler_end_repeat,time_in_second_end*1000000);
    }
    else
    {
        start_timer.callback = timer_on_rellay_only_one;
        esp_timer_create(&start_timer,&pin->timers.timer_handler_begin);
        esp_timer_start_once(pin->timers.timer_handler_begin,time_in_second_begin*1000000);
        stop_timer.callback = timer_off_rellay_only_one;
        esp_timer_create(&stop_timer,&pin->timers.timer_handler_end);
        esp_timer_start_once(pin->timers.timer_handler_end,time_in_second_end*1000000);
    }
}
static void timer_on_rellay_only_one(void *arg)
{
    pin_state* pin = (pin_state*)arg;
    printf("Sa pornit releul\n");
    set_state(pin,true);
}
static void timer_off_rellay_only_one(void *arg)
{
    pin_state* pin = (pin_state*)arg;
    printf("Sa stins releul\n");
    set_state(pin,false);
}
static void timer_on_rellay(void *arg)
{
    pin_state* pin = (pin_state*)arg;
    time_t interval = 86400000000*pin->timers.repeat;
    esp_timer_stop(pin->timers.timer_handler_begin_repeat);
    esp_timer_create_args_t start_timer = 
    {
        .name = "Start",
        .callback = timer_on_rellay,
        .arg = pin
    };
    esp_timer_create(&start_timer,&pin->timers.timer_handler_begin_repeat);
    esp_timer_start_periodic(pin->timers.timer_handler_begin_repeat,interval);
    set_state(pin,true);
    ESP_LOGW(TAG,"enable esire, timer %lld", interval);
}
static void timer_off_rellay(void* arg)
{
    pin_state* pin = (pin_state*)arg;
    time_t interval = 86400000000*pin->timers.repeat;
    esp_timer_stop(pin->timers.timer_handler_end_repeat);
    esp_timer_create_args_t start_timer = 
    {
        .name = "Stop",
        .callback = timer_off_rellay,
        .arg = pin
    };
    esp_timer_create(&start_timer,&pin->timers.timer_handler_end_repeat);
    esp_timer_start_periodic(pin->timers.timer_handler_end_repeat,interval);
    set_state(pin,false);
    ESP_LOGW(TAG,"disable esire, timer %lld", interval);
}
void prase_data_form_time(pin_state* pin, char* data,bool choose_time,uint16_t lenght)
{
    printf("%s\n",data);
    uint32_t i=7;
    uint32_t contur=0;
    char time_char[100];
    memset(time_char,'\000',100);
    time_t *begin_timer_from_pin;
    time_t *end_timer_from_pin;
    if(choose_time)
    {
        printf("Este true \n");
        begin_timer_from_pin = &pin->timers.time_begin_repeat;
        end_timer_from_pin = &pin->timers.time_end_repeat;
    }
    else
    {
        printf("Este false \n");
        begin_timer_from_pin = &pin->timers.time_begin;
        end_timer_from_pin = &pin->timers.time_end;
    }
    while(data[i] != ',')
    {
        time_char[contur] = data[i];
        i++;
        contur++;
    }
    i++;
    contur = 0;
    current_time = atoll(time_char)+10800;
    ESP_LOGW(TAG,"Current Time: %lld",current_time);
    memset(time_char,'\000',100);
    while(data[i] != ',')
    {
        time_char[contur] = data[i];
        i++;
        contur++;
    }
    i++;
    contur = 0;
    *begin_timer_from_pin = atoll(time_char)+10800;
    ESP_LOGW(TAG,"Time inceput: %lld",*begin_timer_from_pin);
    memset(time_char,'\000',100);
    if(choose_time)
    {
        while(data[i] != ',')
        {
            time_char[contur] = data[i];
            i++;
            contur++;
        }
    }
    else
    {
        while(i<lenght)
        {
            time_char[contur] = data[i];
            i++;
            contur++;
        }
    }
    *end_timer_from_pin = atoll(time_char)+10800;
    ESP_LOGW(TAG,"Time sfarsit: %lld", *end_timer_from_pin);

    if(choose_time){
    memset(time_char,'\000',100);
        i++;
        contur = 0;
        time_char[contur] = data[i];
        pin->timers.repeat = atoll(time_char);
        ESP_LOGW(TAG,"Repeat: %d", pin->timers.repeat);
    }
    inti_time(pin,choose_time);
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
            char data[100];
            memset(data,'\000',100);
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
                    set_state(&pins[0],true);
                    ESP_LOGI(TAG,"Sunt aici Turn ON rellay 1");
                }break;
                case 3:
                {
                    set_state(&pins[0],false);
                    ESP_LOGI(TAG,"Turn OFF rellay 1");
                }break;
                case 4:
                {
                    prase_data_form_time(&pins[0], data,true,ctxt->om->om_len);
                    ESP_LOGI(TAG,"Turn ON with repeat timer rellay 1");
                }break;
                case 5:
                {
                    esp_timer_stop(pins[0].timers.timer_handler_begin_repeat);
                    esp_timer_stop(pins[0].timers.timer_handler_end_repeat);
                    ESP_LOGI(TAG,"TURN OFF repeat timer for rellay 1");
                }break;
                case 6:
                {
                    prase_data_form_time(&pins[0], data,false,ctxt->om->om_len);
                }break;
                case 7:
                {
                    esp_timer_stop(pins[0].timers.timer_handler_begin);
                    esp_timer_stop(pins[0].timers.timer_handler_end);
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
                    prase_data_form_time(&pins[1], data,true,ctxt->om->om_len);
                    ESP_LOGI(TAG,"Turn ON with repeat timer rellay 2");
                }break;
                case 69:
                {
                    esp_timer_stop(pins[1].timers.timer_handler_begin_repeat);
                    esp_timer_stop(pins[1].timers.timer_handler_end_repeat);
                    ESP_LOGI(TAG,"Turn OFF repeat timer for rellay 2");
                }break;
                case 70: 
                {
                    prase_data_form_time(&pins[0], data,true,ctxt->om->om_len);
                    ESP_LOGI(TAG,"Turn ON with only one timer rellay 2");
                }break;
                case 71: 
                {
                    esp_timer_stop(pins[0].timers.timer_handler_begin);
                    esp_timer_stop(pins[0].timers.timer_handler_end);  
                    ESP_LOGI(TAG,"Turn OFF with timer rellay 2");               
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
                    ESP_LOGI(TAG,"Error de aici");
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
    //  inti_time(&pins[0]);
    // esp_timer_handle_t handler;
    // esp_timer_create_args_t asp = 
    // {
    //     .callback = timer,
    //     .name = "abc"
    // };
    // esp_timer_create(&asp,&handler);
    // esp_timer_start_periodic(handler,30000000);
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