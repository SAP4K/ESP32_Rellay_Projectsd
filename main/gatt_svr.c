
/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "bleprph.h"
#include "services/ans/ble_svc_ans.h"
#include <driver/gpio.h>

static void init_pins()
{
    gpio_set_direction(pins[0].pin,GPIO_MODE_OUTPUT);
    gpio_set_direction(pins[1].pin,GPIO_MODE_OUTPUT);
}
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt,void *arg);
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                .uuid = BLE_UUID16_DECLARE(0xFEEF4),
                .access_cb = gatt_svc_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            }, 
            {0,}
        },
    },
    {0,},
};
uint8_t check_recived_data(char* data)
{
    return 0;
}
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc=0;
    char send[] = "testssss";
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
    {
        rc = os_mbuf_append(ctxt->om,send,sizeof(send));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        {
            char* data = (char*)calloc(ctxt->om->om_len, sizeof(char));;
            //memset(data,'\n',ctxt->om->om_len);
            ble_hs_mbuf_to_flat(ctxt->om,data,ctxt->om->om_len,NULL);
            ESP_LOGI("Personal","%s",data);
            //check_recived_data(data);
            free(data);
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

int gatt_svr_init(void)
{
    init_pins();
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
