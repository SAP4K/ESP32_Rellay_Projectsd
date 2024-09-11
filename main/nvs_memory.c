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
     rc = nvs_get_u8(handler,CHECK_USER,&value);
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
esp_err_t nvs_get_user_id(uint32_t* get_user_id)
{
     int rc;
     nvs_handle_t handler;
     uint32_t user;
     rc = nvs_set_handler(&handler,ID_USER);
     if(rc != ESP_OK)
          return rc;
     rc = nvs_get_u32(handler,USER,&user);
     if(get_user_id != NULL)
     {
          *get_user_id = user;
          ESP_LOGE("NVS","Diferit de zero");
     }
     else
     {
          ESP_LOGE("NVS","EGAL CU ZERO");
     }
     if(rc != ESP_OK)
          return rc;
     return ESP_OK;
}
esp_err_t nvs_set_user_id(uint32_t* user_id)
{
     int rc;
     nvs_handle_t handler;
     rc = nvs_set_handler(&handler,ID_USER);
     if(rc != ESP_OK)
          return rc;
     uint32_t user;
     ESP_LOGE("NVS","Set user with ID: %ld",*user_id);
     rc = nvs_set_u32(handler,USER,*user_id);
     if(rc != ESP_OK)
          return rc;
     ESP_LOGE("NVS","A fost setat userul");
     return ESP_OK;
}