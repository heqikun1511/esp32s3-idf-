/**
 * LVGL image descriptors for PNG-compressed images.
 *
 * Uses LV_COLOR_FORMAT_UNKNOWN so LVGL's built-in lodepng decoder
 * will decode the PNG data on-demand into PSRAM.
 * Decoded images are cached by LVGL for subsequent renders.
 *
 * This replaces the pixel-data arrays in the original ui_image_*.c files,
 * which are excluded from compilation to reduce firmware size (~42 MB -> ~1.4 MB).
 */
#include "lvgl.h"
#include "png_images.h"

/* ------------------------------------------------------------------ */
/*  ami_ui_board   1920 x 1080                                        */
/* ------------------------------------------------------------------ */
lv_image_dsc_t img_ami_ui_board = {
    .header = {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_UNKNOWN,
        .flags = 0,
        .w = 1920,
        .h = 1080,
        .stride = 0,
        .reserved_2 = 0,
    },
    .data_size = png_ami_ui_board_size,
    .data = png_ami_ui_board_data,
    .reserved = NULL,
};

/* ------------------------------------------------------------------ */
/*  foll_picture   1920 x 1080                                        */
/* ------------------------------------------------------------------ */
lv_image_dsc_t img_foll_picture = {
    .header = {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_UNKNOWN,
        .flags = 0,
        .w = 1920,
        .h = 1080,
        .stride = 0,
        .reserved_2 = 0,
    },
    .data_size = png_foll_picture_size,
    .data = png_foll_picture_data,
    .reserved = NULL,
};

/* ------------------------------------------------------------------ */
/*  eight_picture  1920 x 1080                                        */
/* ------------------------------------------------------------------ */
lv_image_dsc_t img_eight_picture = {
    .header = {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_UNKNOWN,
        .flags = 0,
        .w = 1920,
        .h = 1080,
        .stride = 0,
        .reserved_2 = 0,
    },
    .data_size = png_eight_picture_size,
    .data = png_eight_picture_data,
    .reserved = NULL,
};

/* ------------------------------------------------------------------ */
/*  acc_picture    1920 x 1080                                        */
/* ------------------------------------------------------------------ */
lv_image_dsc_t img_acc_picture = {
    .header = {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_UNKNOWN,
        .flags = 0,
        .w = 1920,
        .h = 1080,
        .stride = 0,
        .reserved_2 = 0,
    },
    .data_size = png_acc_picture_size,
    .data = png_acc_picture_data,
    .reserved = NULL,
};
