set(srcs "main.c"
         "gatt_svr.c")

idf_component_register(SRCS "adc.c" "nvs_memory.c" "config_time.c" "${srcs}"
                       INCLUDE_DIRS ".")
