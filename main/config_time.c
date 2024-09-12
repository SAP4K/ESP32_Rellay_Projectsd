#include "config_time.h"
static QueueHandle_t gpio_evt_queue = NULL;
void call_back()
{
     ESP_LOGE("TIM","Se notifica");
     xQueueSendFromISR(gpio_evt_queue,NULL,NULL);
}
void timer_task()
{
     while(true)
     {
          if(xQueueReceive(gpio_evt_queue,NULL,portMAX_DELAY))
          {
               ESP_LOGE("TIM","Sunt in Queue");
          }
     }
}
node_castom_timer* bottom()
{
     
     return NULL;
}
void push(node_castom_timer nod)
{
     node_castom_timer* next_nod = (node_castom_timer*)malloc(sizeof(node_castom_timer));
     *next_nod = nod;
     ESP_LOGE("TIM","push: %lld",next_nod->timer.data_set_time);
     ESP_LOGE("TIM","push: %lld",next_nod->timer.seconds_timer_begin);
     ESP_LOGE("TIM","push: %lld",next_nod->timer.seconds_timer_end);
     ESP_LOGE("TIM","push: %ld", next_nod->id_timer);
     ESP_LOGE("TIM","push: %c",  next_nod->timer.timer_type);
}
void pop()
{

}
node_castom_timer tim_parse_data(char* data)
{
     char* buffer;
     char string_buffer[100];
     node_castom_timer nod;
     size_t position = 0;
     nod.Node = NULL;
     memset(string_buffer,'\000',sizeof(string_buffer));
     buffer = strchr(data,',');
     position = (size_t)(buffer-data);
     memcpy(string_buffer,&data[0],position);
     nod.timer.data_set_time = atoll(string_buffer);
     buffer = strchr(buffer+1,',');
     memset(string_buffer,'\000',sizeof(string_buffer));
     memcpy(string_buffer,&data[position+1],(size_t)(buffer-data-position-1));
     nod.timer.seconds_timer_begin = atoll(string_buffer);
     position = (size_t)(buffer-data);
     buffer = strchr(buffer+1,',');
     memset(string_buffer,'\000',sizeof(string_buffer));
     memcpy(string_buffer,&data[position+1],(size_t)(buffer-data-position-1));
     nod.timer.seconds_timer_end = atoll(string_buffer);
     position = (size_t)(buffer-data);
     buffer = strchr(buffer+1,',');
     memset(string_buffer,'\000',sizeof(string_buffer));
     memcpy(string_buffer,&data[position+1],(size_t)(buffer-data-position-1));
     nod.id_timer = atol(string_buffer);
     nod.timer.timer_type = data[strlen(data)-1];

     return nod;
}
void tim_set_timer(char* data)
{
     ESP_LOGE("TIM","%s",data);
     node_castom_timer nod = tim_parse_data(data);
     push(nod);
}
void tim_init()
{
     ESP_LOGE("TIM","Se initializeaza timerul");
     gpio_evt_queue = xQueueCreate(configSUPPORT_DYNAMIC_ALLOCATION,NULL);
     xTaskCreate(timer_task,"test",2048,NULL,10,NULL);
}