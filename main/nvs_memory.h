#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#define BLE_ADDRESS "Storage_BLE"
#define ID_USER "ID_User"
#define USER "USER"
#define CHECK_USER "ID1"
esp_err_t nvs_check_mac_address();
esp_err_t nvs_write_mac_address(uint8_t*);
esp_err_t nvs_read_mac_address(uint8_t*);
esp_err_t nvs_get_user_id(uint32_t*);
esp_err_t nvs_set_user_id(uint32_t*);