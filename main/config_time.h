#include "esp_log.h"
#include "esp_timer.h"
typedef struct castom_timer
{
     esp_timer_handle_t timer_begin;
     esp_timer_handle_t timer_end;
     time_t seconds_timer_begin;
     time_t seconds_timer_end;
     char timer_type;
}castom_timer;

typedef struct node_castom_timer
{
     castom_timer timer;
     uint32_t id_timer;
     struct node_castom_timer* Node;
}node_castom_timer;

void set_timer();