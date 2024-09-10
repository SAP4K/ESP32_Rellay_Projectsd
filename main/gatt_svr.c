#include "bleprph.h"
#define TAG "Personal"
bool testam_alata_denumire = false;
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt,void *arg);
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
    if(nr_relay > 71)
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
//////////////////////////////////////////////////

////////////////////////////////////////////////// 
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
    //inti_time(pin,choose_time);
}

static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc=0;
    static char send[30] = {0};
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
            if(testam_alata_denumire)
            {
                ESP_LOGE(TAG,"ESTE true");
            }
            else
            {
                ESP_LOGE(TAG,"ESTE false");
            }
            if(testam_alata_denumire){
            switch(check_recived_data(data))
            {
                case 1:
                {
                    if(get_state(&pins[0]))
                    {
                        strcpy(send,"1_ON");
                        printf("citire stare ON reallay 1\n");
                        characters = 5;
                    }
                    else
                    {
                        strcpy(send,"1_OFF");
                        printf("citire stare OFF reallay 1\n");
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
                    /*Timer pentru releul unu cu repetare*/
                    ESP_LOGI(TAG,"Turn ON with repeat timer rellay 1");
                }break;
                case 5:
                {
                    /*Oprire timer pentru primul releu cu repetare*/
                    ESP_LOGI(TAG,"TURN OFF repeat timer for rellay 1");
                }break;
                case 6:
                {
                    /*Pornire timer pentru primul releu fara repetare*/
                    prase_data_form_time(&pins[0], data,false,ctxt->om->om_len);
                    ESP_LOGI(TAG,"Turn ON with only one timer rellay 1");
                }break;
                case 7:
                {
                    /*Oprire timer pentru primul releu fara repetare*/
                }break;
                case 65:
                {
                    if(get_state(&pins[1]))
                    {
                        strcpy(send,"2_ON");
                        printf("citire stare ON reallay 2\n");
                        characters = 5;
                    }
                    else
                    {
                        strcpy(send,"2_OFF");
                        printf("citire stare OFF reallay 1\n");
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
                    /*Timer pentru releul doi cu repetare*/
                    printf("Sunt aici rellay 2 run timer\n");
                    prase_data_form_time(&pins[1], data,true,ctxt->om->om_len);
                    ESP_LOGI(TAG,"Turn ON with repeat timer rellay 2");
                }break;
                case 69:
                {
                     /*Oprire timer pentru al doilea releu cu repetare*/
                }break;
                case 70: 
                {
                    /*Pornire timer pentru al doilea releu fara repetare*/
                    prase_data_form_time(&pins[1], data,false,ctxt->om->om_len);
                    ESP_LOGI(TAG,"Turn ON with only one timer rellay 2");
                }break;
                case 71: 
                {
                }break;
                case 99:
                {
                        int battery_level = adc_get_battery();
                        itoa(battery_level,send,10);
                        if(battery_level>=100)
                        {
                            characters = 3;
                        }
                        if(battery_level>=10 && battery_level<100)
                        {
                            characters = 2;
                        }
                        if(battery_level<10)
                        {
                          characters = 1;  
                        }
                        ESP_LOGI("ADC","In string: %s",send);
                        ESP_LOGI("ADC","Nr caractere: %d",characters);
                }break;
                default:
                {
                    ESP_LOGI(TAG,"Error de aici");
                }break;
            }    
            }
            else
            {  
                bool check_message = false;
                char dates[50];
                memset(dates,'\000',sizeof(dates));
                strncpy(dates,data,6);
                if(strcmp(dates,"UX0000") != 0)
                {
                    memset(send,'\000',sizeof(send));
                    strcpy(send,"COMAND_ERROR");
                    characters = (uint8_t)strlen(send);
                }
                else
                    check_message = true;

                if(check_message)
                {
                    rc = nvs_get_user_id(NULL);
                    uint32_t user = (uint32_t)atoi(&data[7]);
                    if(rc != ESP_OK)
                    {
                        rc = nvs_set_user_id(&user);
                        if(rc != ESP_OK)
                            return rc;
                        testam_alata_denumire = true;
                        memset(send,'\000',sizeof(send));
                        strcpy(send,"YES");
                        characters = strlen(send);
                    }
                    else
                    {
                        uint32_t user_id;
                        nvs_get_user_id(&user_id);
                        memset(send,'\000',sizeof(send));
                        if(user_id == user)
                        {
                            testam_alata_denumire = true;
                            strcpy(send,"YES");
                        }
                        else
                        {
                            ESP_LOGE("LOGARE","Nu este user");
                            strcpy(send,"NO");
                        }
                        characters = strlen(send);
                    }
                }
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
int gatt_svr_init(void)
{
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