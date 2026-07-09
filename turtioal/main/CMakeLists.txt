idf_component_register(SRCS "main.c"
                    "../BSP/LED/led.c"
                    "../BSP/KEY/key.c"
                    "../BSP/EXIT/exti.c"
                    "../BSP/UART/uart.c"
                    INCLUDE_DIRS "." ".."
                    "../BSP/LED"
                    "../BSP/KEY"
                    "../BSP/EXIT"
                    "../BSP/UART"
                    "../BSP/esptim"
                    )
#官方提供的函数，告诉系统那个文件需要编译，那个文件需要包含头文件，那个文件需要依赖其他组件