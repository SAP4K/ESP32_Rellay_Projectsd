#include "nvs_memory.h"
#include "string.h"
esp_err_t nvs_set_handler(nvs_handle_t* handler,const char* meesage)
{
     int rc;
     rc = nvs_open(meesage,NVS_READWRITE,handler);
     if(rc != ESP_OK)
          return rc;

     return ESP_OK;
}
esp_err_t nvs_check_mac_address()
{
     int rc;
     nvs_handle_t handler;
     rc = nvs_set_handler(&handler,BLE_ADDRESS);
     if(rc != ESP_OK)
          return rc;
     uint8_t value;
     rc = nvs_get_u8(handler,"ID1",&value);
     if(rc != ESP_OK)
          return rc;
     nvs_close(handler);
     return ESP_OK;
}
esp_err_t nvs_write_mac_address(uint8_t* address)
{
     int rc;
     nvs_handle_t handler;
     char ID[4];
     memset(ID,'\000',sizeof(ID));
     strcpy(ID,"ID");
     rc = nvs_set_handler(&handler,BLE_ADDRESS);
     if(rc != ESP_OK)
          return rc;
     ESP_LOGE("NVS","Se scrie in memorie");
     for(uint8_t i=0;i<6;i++)
     {
          ID[2] = (char)(i+49);
          ESP_LOGE("NVS","%02x",address[i]);
          nvs_set_u8(handler,ID,address[i]);
     }
     return ESP_OK;
}
esp_err_t nvs_read_mac_address(uint8_t* address)
{
     int rc;
     nvs_handle_t handler;
     char ID[4];
     memset(ID,'\000',sizeof(ID));
     strcpy(ID,"ID");
     rc = nvs_set_handler(&handler,BLE_ADDRESS);
     if(rc != ESP_OK)
          return rc;
     for(uint8_t i=0;i<6;i++)
     {
          ID[2] = (char)(i+49);
          nvs_get_u8(handler,ID,&address[i]);
     }

     return ESP_OK;
}
esp_err_t nvs_check_user()
{
     int rc;
     nvs_handle_t handler;
     rc = nvs_set_handler(&handler,ID_USER);
     if(rc != ESP_OK)
          return rc;
     char user_id[5];
     memset(user_id,'\000',sizeof(user_id));
     strcpy(user_id,"USER");
     uint32_t user;
     rc = nvs_get_u32(handler,user_id,&user);
     if(rc != ESP_OK)
          return rc;
     return ESP_OK;
}
esp_err_t nvs_set_user_id()
{

     return ESP_OK;
}