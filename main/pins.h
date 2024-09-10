#include "driver/gpio.h"
typedef struct pin_state
{
 gpio_num_t pin;
 uint32_t state;
 bool rellay_state;
} pin_state;
static pin_state pins[2] = 
{
{.pin=GPIO_NUM_3, .state=false, },
{.pin=GPIO_NUM_2, .state=false, }
};
esp_err_t pin_init_pins();
bool pin_get_state(const pin_state*);
esp_err_t pin_set_state(pin_state*,bool);