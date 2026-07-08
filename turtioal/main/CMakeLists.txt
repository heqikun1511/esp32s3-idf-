idf_component_register(SRCS "main.c"
                    "../BSP/LED/led.c"
                    "../BSP/KEY/key.c"
                    "../BSP/EXIT/exti.c"
                    INCLUDE_DIRS "." ".."
                    "../BSP/LED"
                    "../BSP/KEY"
                    "../BSP/EXIT")
