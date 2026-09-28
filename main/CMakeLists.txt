idf_component_register(
    SRC_DIRS 
        "."
        "APP" 
        "ui"
        "BSP" 
    INCLUDE_DIRS 
        "."
        "APP"
        "ui"
        "BSP"
    REQUIRES
        driver
        esp_driver_rmt
        nvs_flash
        esp_timer
        BSP)

# Compatibility implementations for EEZ native variables referenced by screens.c.
target_sources(${COMPONENT_LIB} PRIVATE "ui_compat.c")
