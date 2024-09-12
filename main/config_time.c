#include "config_time.h"
static QueueHandle_t queue_enable_output = NULL;
static QueueHandle_t queue_desable_output = NULL;
void tim_callback_enable_output(void* arg)
{
     pin_state* pin = (node_castom_timer*)arg;
     xQueueSendFromISR(queue_enable_output,pin,NULL);
}

void tim_callback_desable_output(void* arg)
{
     pin_state* pin = (node_castom_timer*)arg;
     xQueueSendFromISR(queue_desable_output,pin,NULL);
}
void tim_enable_task()
{
     node_castom_timer node;
     while(true)
     {
          if(xQueueReceive(queue_enable_output,&node,portMAX_DELAY))
          {
               ESP_LOGE("TIM","run output");
          }
     }
}
void tim_disable_task()
{
     node_castom_timer node;
     while(true)
     {
          if(xQueueReceive(queue_desable_output,&node,portMAX_DELAY))
          {
               ESP_LOGE("TIM","stop output");
          }
     }
}
node_castom_timer* tim_bottom()
{
     
     return NULL;
}
node_castom_timer* tim_get_last_element()
{
     node_castom_timer* get_last_element = head;
     while((get_last_element->Node != NULL) )
     {
          get_last_element = get_last_element->Node;
     }
     return get_last_element;
}
void tim_push(node_castom_timer nod)
{
     node_castom_timer* next_nod = (node_castom_timer*)malloc(sizeof(node_castom_timer));
     *next_nod = nod;
     if(head == NULL)
     {
          head = next_nod;
          return;
     }
     node_castom_timer* get_last_element = tim_get_last_element();
     get_last_element->Node = next_nod;
}
void tim_pop()
{

}
void tim_run_timer()
{
     node_castom_timer* timer_for_run = tim_get_last_element();
     ESP_LOGE("TIM","RUN timers");
     time_t begin_time = timer_for_run->timer.seconds_timer_begin - timer_for_run->timer.data_set_time;
     time_t end_time = timer_for_run->timer.seconds_timer_end - timer_for_run->timer.data_set_time;
     esp_timer_create_args_t timer = 
     {
          .name = "Start_Timer",
          .callback = tim_callback_enable_output,
          .arg = timer_for_run
     };
     esp_timer_create(&timer,&timer_for_run->timer.timer_begin);
     esp_timer_start_periodic(timer_for_run->timer.timer_begin,begin_time);
     timer.name = "End_Timer";
     timer.callback = tim_callback_desable_output;
     timer.arg = timer_for_run;
     esp_timer_create(&timer,&timer_for_run->timer.timer_end);
     esp_timer_start_periodic(timer_for_run->timer.timer_end,end_time);
}
node_castom_timer tim_parse_data(char* data)
{    
     char* buffer;
     char string_buffer[BUFFER_SIZE];
     node_castom_timer nod;
     size_t position = 0;
     nod.Node = NULL;
     memset(string_buffer,'\000',sizeof(string_buffer));
     buffer = strchr(data,',');
     position = (size_t)(buffer-data);
     memcpy(string_buffer,&data[0],position);
     nod.timer.data_set_time = atoll(string_buffer);
     for(uint8_t i=0;i<3;i++)
     {
          buffer = strchr(buffer+1,',');
          memset(string_buffer,'\000',sizeof(string_buffer));
          memcpy(string_buffer,&data[position+1],(size_t)(buffer-data-position-1));
          switch (i)
          {
               case 0:{nod.timer.seconds_timer_begin = atoll(string_buffer);}break;
               case 1:{nod.timer.seconds_timer_end = atoll(string_buffer);}break;
               case 2:{nod.id_timer = atol(string_buffer);}break;
          }
          position = (size_t)(buffer-data);
     }
     nod.id_timer = atol(string_buffer);
     nod.timer.timer_type = data[strlen(data)-1];
     return nod;
}
esp_err_t tim_set_timer(char* data,pin_state* pin)
{
     
     size_t lenght =  strlen(data);
     if(lenght<36)
          return ESP_ERR_INVALID_SIZE;
     node_castom_timer nod = tim_parse_data(data);
     nod.timer.pin = pin;
     tim_push(nod);
     tim_run_timer();
     return ESP_OK;
}
void tim_init()
{
     ESP_LOGE("TIM","Se initializeaza timerul");
     queue_enable_output =  xQueueCreate(configSUPPORT_DYNAMIC_ALLOCATION,sizeof(node_castom_timer));
     queue_desable_output = xQueueCreate(configSUPPORT_DYNAMIC_ALLOCATION,sizeof(node_castom_timer));
     xTaskCreate(tim_enable_task,  "enable",2048,NULL,10,NULL);
     xTaskCreate(tim_disable_task,"disable",2048,NULL,10,NULL);
}