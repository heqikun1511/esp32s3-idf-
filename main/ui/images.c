#include "images.h"

/*
 * The EEZ export references an image named "main", but its generated image
 * source was not included in this project. Keep the exported symbol valid so
 * the UI can run without that optional background asset.
 */
static const uint8_t img_main_map[] = { 0x00, 0x00, 0x00, 0x00 };

const lv_img_dsc_t img_main = {
    .header.always_zero = 0,
    .header.w = 1,
    .header.h = 1,
    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
    .data_size = sizeof(img_main_map),
    .data = img_main_map,
};

const ext_img_desc_t images[1] = {
    { "main", &img_main },
};
