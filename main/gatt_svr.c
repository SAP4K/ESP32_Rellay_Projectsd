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
    ESP_LOGI(TAG,"%s",data);
    switch(data[0])
    {
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
    char comapre[4];
    memcpy(comapre,&data[2],3);
    comapre[3] = '\000';
    if(strcmp("000",comapre) != 0)
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
                    set_state(&pins[0],true);
                    ESP_LOGI(TAG,"Turn ON rellay 1");
                }break;
                case 3:
                {
                    set_state(&pins[0],false);
                    ESP_LOGI(TAG,"Turn OFF rellay 1");
                }break;
                case 4:
                {
                    ESP_LOGI(TAG,"Turn ON with timer rellay 1");
                }break;
                case 5:
                {
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
                    set_state(&pins[1],true);
                    ESP_LOGI(TAG,"Turn ON rellay 2");     
                }break;
                case 67:
                {
                    set_state(&pins[1],false);
                    ESP_LOGI(TAG,"Turn OFF rellay 2");
                }break;
                case 68:
                {
                    ESP_LOGI(TAG,"Turn ON with timer rellay 1");
                }break;
                case 69:
                {
                    ESP_LOGI(TAG,"Turn OFF timer for rellay 2");
                }break;
                case 99:
                {
                    ESP_LOGI(TAG,"Battery level");
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

int
gatt_svr_init(void)
{
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

    /* Setting a value for the read-only descriptor */


    return 0;
}
