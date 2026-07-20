#include "png_images.h"

const png_image_t png_images[] = {
    { "ami_ui_board", png_ami_ui_board_data, png_ami_ui_board_size, png_ami_ui_board_w, png_ami_ui_board_h },
    { "foll_picture", png_foll_picture_data, png_foll_picture_size, png_foll_picture_w, png_foll_picture_h },
    { "eight_picture", png_eight_picture_data, png_eight_picture_size, png_eight_picture_w, png_eight_picture_h },
    { "acc_picture", png_acc_picture_data, png_acc_picture_size, png_acc_picture_w, png_acc_picture_h },
};

const int png_image_count = sizeof(png_images) / sizeof(png_images[0]);
