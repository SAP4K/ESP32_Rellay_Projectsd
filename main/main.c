/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "bleprph.h"
static const char *tag = "NimBLE_BLE_PRPH";
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;
nvs_handle_t memory_handler;
void ble_store_config_init(void);
static uint8_t test2[3] = {123,255,97};
static void bleprph_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    memset(&fields, 0, sizeof fields);

    
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;
    fields.svc_data_uuid16 = test2;
    fields.svc_data_uuid16_len = (uint8_t)sizeof(test2);
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

    
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(500);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(5000);
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
        MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
        }
        MODLOG_DFLT(INFO, "\n");
        if (event->connect.status != 0) {
            
            bleprph_advertise();
        }
        return 0;
    case BLE_GAP_EVENT_DISCONNECT:
        {
        extern bool testam_alata_denumire;
        printf("Deconectare\n");
        printf("\nAdress: %p\n",(void*)&memory_handler);
        testam_alata_denumire = false;
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        MODLOG_DFLT(INFO, "\n");
        
        bleprph_advertise();
        return 0;
        }
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }
    return 0;
}
static void bleprph_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}
static void bleprph_on_sync(void)
{
    int rc;
    assert(rc == 0);
    esp_err_t respons = nvs_check_mac_address();
    if(respons != ESP_OK)
    {
        ESP_LOGE("BLE","Nu este adresa");
        ble_hs_id_gen_rnd(0,&addr);
        nvs_write_mac_address(&addr.val);
    }
    else
    {
        ESP_LOGE("BLE","Este adresa");
        nvs_read_mac_address(&addr.val);
    }
    /* Figure out address to use while advertising (no privacy for now) */
    ble_hs_id_set_rnd(addr.val);
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
    
    nimble_port_run();

    nimble_port_freertos_deinit();
}
static void init_pins()
{
    gpio_set_direction(pins[0].pin,GPIO_MODE_OUTPUT);
    gpio_set_direction(pins[1].pin,GPIO_MODE_OUTPUT);
    gpio_set_level(pins[0].pin,0);
    gpio_set_level(pins[1].pin,0);
    gpio_hold_en(pins[0].pin);
    gpio_hold_en(pins[1].pin);
}
void app_main(void)
{
    int rc;
    init_pins();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGE("Personalizat","Curatire memorie");
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
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    rc = gatt_svr_init();
    assert(rc == 0);
    rc = ble_svc_gap_device_name_set("EBS2");
    assert(rc == 0);
    ble_store_config_init();
    nimble_port_freertos_init(bleprph_host_task);
}