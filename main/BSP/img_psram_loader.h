#ifndef IMG_PSRAM_LOADER_H
#define IMG_PSRAM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 将所有图片数据从 flash 拷贝到 PSRAM，
 *        并更新 lv_image_dsc_t 的 data 指针指向 PSRAM。
 * 
 * 解决固件体积过大超过 flash 限制的问题。
 * 注意：各图片的 map[] 数组定义中去掉了 static，
 * 且 lv_image_dsc_t 去掉了 const 以便运行时修改指针。
 * 
 * @return 成功拷贝的图片数量
 */
int img_psram_load_all(void);

#ifdef __cplusplus
}
#endif

#endif /* IMG_PSRAM_LOADER_H */
