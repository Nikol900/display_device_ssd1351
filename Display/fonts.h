#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

typedef struct {
    const uint8_t width;
    uint8_t height;
    uint8_t yShift;
    const uint16_t *data;
    const int16_t *symbMatch;
} FontType;



extern FontType Font_4_Normal;
extern FontType Font_4_Underlined;
extern FontType Font_5_Normal;
extern FontType Font_5_Underlined;
extern FontType Font_6_Normal;
extern FontType Font_6_Underlined;


#endif // __FONTS_H__
