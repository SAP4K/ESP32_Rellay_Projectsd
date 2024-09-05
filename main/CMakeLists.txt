set(srcs "main.c"
         "gatt_svr.c")

idf_component_register(SRCS "nvs_memory.c" "config_time.c" "${srcs}"
                       INCLUDE_DIRS ".")
