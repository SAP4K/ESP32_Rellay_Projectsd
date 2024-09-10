#include "pins.h"
esp_err_t pin_init_pins()
{
     int rc;
     for(uint8_t i=0;i<2;i++){
          rc = gpio_set_direction(pins[i].pin,GPIO_MODE_OUTPUT);
          if(rc != ESP_OK)
               return rc;
          rc = gpio_set_level(pins[i].pin,0);
          if(rc != ESP_OK)
               return rc;
          rc = gpio_hold_en(pins[i].pin);
          if(rc != ESP_OK)
               return rc;
    }
    return ESP_OK;
}

bool pin_get_state(const pin_state* pin)
{
    if(!pin->state)
        return false;

    return true;
}
esp_err_t pin_set_state(pin_state* pin,bool state)
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
    return ESP_OK;
}