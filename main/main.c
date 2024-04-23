/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "nvs_flash.h"
/* power management */
#include "esp_pm.h"
#include "esp_sleep.h"
/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include <host/ble_gap.h>
#include <host/ble_hs.h>
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "bleprph.h"
static const char *tag = "NimBLE_BLE_PRPH";
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;
void ble_store_config_init(void);

static void bleprph_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    memset(&fields, 0, sizeof fields);


    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_LEN;
    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;
    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;
    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }
    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
   // adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(500);
   // adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(1000);
    

  //  struct ble_gap_ext_adv_params params;
  //  memset(&params, 0, sizeof(params));
  //  params.own_addr_type = own_addr_type,
 //   params.primary_phy = BLE_HCI_LE_PHY_CODED,
 //   params.secondary_phy = BLE_HCI_LE_PHY_CODED;
 //   params.legacy_pdu = 0;
 //   params.tx_power = 127;
 //   params.sid = 0;
 //   int8_t tx_power;
 //   int res = ble_gap_ext_adv_configure(1, &params, NULL, _on_gap_evt, NULL);


    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, NULL);  
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}
static int bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
        }
        MODLOG_DFLT(INFO, "\n");
        if (event->connect.status != 0) {
            /* Connection failed; resume advertising. */
            bleprph_advertise();
        }
        return 0;
    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        MODLOG_DFLT(INFO, "\n");
        /* Connection terminated; resume advertising. */
        bleprph_advertise();
        return 0;
        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }
    return 0;
}
static void bleprph_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}
static uint8_t ID_Dispozitiv_blutooth[6] = {0x11,0x11,0x11,0x11,0x11,0x11};
static void bleprph_on_sync(void)
{
    int rc;
    ble_hs_id_set_rnd(ID_Dispozitiv_blutooth);
    ble_hs_id_copy_addr(BLE_ADDR_PUBLIC,NULL,NULL);
    assert(rc == 0);
    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }
    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");
    /* Begin advertising. */
    bleprph_advertise();
}

void bleprph_host_task(void *param)
{
    //ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    ESP_LOGI("Personal", "Basic func");
    nimble_port_run();

    nimble_port_freertos_deinit();
}
void app_main(void)
{
    int rc;
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGI("Personalizat","Curatire memorie");
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_pm_config_t pm_config = {
            .max_freq_mhz = CONFIG_EXAMPLE_MAX_CPU_FREQ_MHZ,
            .min_freq_mhz = CONFIG_EXAMPLE_MIN_CPU_FREQ_MHZ,
            .light_sleep_enable = true
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );

    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(tag, "Failed to init nimble %d ", ret);
        return;
    }
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
 
    rc = gatt_svr_init();
    assert(rc == 0);
    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("nimble-bleprph");
    assert(rc == 0);
    /* XXX Need to have template for store */
    ble_store_config_init();
    nimble_port_freertos_init(bleprph_host_task);
    /* Initialize command line interface to accept input from user */
    //rc = scli_init();
}
