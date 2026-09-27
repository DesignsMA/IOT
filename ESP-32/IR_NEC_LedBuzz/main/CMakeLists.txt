idf_component_register(SRCS "main.c"
                INCLUDE_DIRS "."
                REQUIRES freertos esp_driver_gpio htcw_rmt_ir esp_system )
