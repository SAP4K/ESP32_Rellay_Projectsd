#include "esp_log.h"
#include "esp_timer.h"
typedef struct personal_timer
{
 esp_timer_handle_t timer_handler_begin_repeat;
 esp_timer_handle_t timer_handler_end_repeat;
 esp_timer_handle_t timer_handler_begin;
 esp_timer_handle_t timer_handler_end;
 time_t time_begin_repeat;
 time_t time_end_repeat;
 time_t time_begin;
 time_t time_end;
 uint8_t repeat;
 bool status_repeat;
 bool status_one[2];
}personal_timer;

void test();