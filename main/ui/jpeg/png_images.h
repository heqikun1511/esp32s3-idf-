#ifndef PNG_IMAGES_H
#define PNG_IMAGES_H

#include <stdint.h>
#include <stddef.h>

#include "ami_ui_board_png.h"
#include "foll_picture_png.h"
#include "eight_picture_png.h"
#include "acc_picture_png.h"

typedef struct {
    const char *name;
    const uint8_t *data;
    size_t size;
    int w;
    int h;
} png_image_t;

extern const png_image_t png_images[];
extern const int png_image_count;

#endif
