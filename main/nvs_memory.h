#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#define SPACE_NAME "Storage_BLE"
esp_err_t nvs_check_mac_address();
esp_err_t nvs_write_mac_address(uint8_t*);
esp_err_t nvs_read_mac_address(uint8_t*);