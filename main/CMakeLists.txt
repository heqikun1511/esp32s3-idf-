idf_component_register(
    SRC_DIRS 
        "."
        "APP" 
        "ui"  
        "ui/jpeg"
        "BSP" 
    INCLUDE_DIRS 
        "."
        "APP"
        "ui"
        "ui/jpeg"
        "BSP"
    EXCLUDE_SRCS
        # 排除原始 ARGB8888 像素数据文件 (巨大 ~42MB),
        # 改用 ui/jpeg/ 中的 JPEG 压缩数据 + 轻量描述符
        "ui/ui_image_ami_ui_board.c"
        "ui/ui_image_foll_picture.c"
        "ui/ui_image_eight_picture.c"
        "ui/ui_image_acc_picture.c"
    REQUIRES
        driver
        nvs_flash
        esp_timer
        BSP)
