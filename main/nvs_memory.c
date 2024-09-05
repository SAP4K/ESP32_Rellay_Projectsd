#include "nvs_memory.h"
#include "string.h"
esp_err_t nvs_set_handler(nvs_handle_t* handler)
{
     int rc;
     rc = nvs_open(SPACE_NAME,NVS_READWRITE,handler);
     if(rc != ESP_OK)
          return rc;

     return ESP_OK;
}
esp_err_t nvs_check_mac_address()
{
     int rc;
     nvs_handle_t handler;
     rc = nvs_set_handler(&handler);
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
     char ID[3] = "ID";
     rc = nvs_set_handler(&handler);
     if(rc != ESP_OK)
          return rc;

     for(uint8_t i=0;i<6;i++)
     {
          ID[2] = (char)(i+49);
          nvs_set_u8(handler,ID,address[i]);
     }
     return ESP_OK;
}
esp_err_t nvs_read_mac_address(uint8_t* address)
{
     int rc;
     nvs_handle_t handler;
     char ID[3] = "ID";
     rc = nvs_set_handler(&handler);
     if(rc != ESP_OK)
          return rc;
     for(uint8_t i=0;i<6;i++)
     {
          ID[2] = (char)(i+49);
          nvs_get_u8(handler,ID,&address[i]);
     }

     return ESP_OK;
}