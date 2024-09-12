#include "esp_log.h"
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "string.h"
#include "pins.h"
#define BUFFER_SIZE 100
typedef struct castom_timer
{
     esp_timer_handle_t timer_begin;
     esp_timer_handle_t timer_end;
     time_t seconds_timer_begin;
     time_t seconds_timer_end;
     time_t data_set_time;
     char timer_type;
     pin_state* pin;
}castom_timer;

typedef struct node_castom_timer
{
     castom_timer timer;
     uint32_t id_timer;
     struct node_castom_timer* Node;
}node_castom_timer;

static node_castom_timer* head=NULL;
void tim_callback_enable_output();
void tim_init();
esp_err_t tim_set_timer(char*,pin_state*);