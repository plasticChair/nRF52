#ifndef PAPER_H
#define PAPER_H

#include "app_error.h"
#include "nrf_lcd.h"
#include "nrf_gfx.h"

extern const nrf_gfx_font_desc_t georgia_6ptFontInfo;
static const nrf_gfx_font_desc_t * p_font = &georgia_6ptFontInfo;

#define PAPER_PIXEL_WIDTH  152
#define PAPER_PIXEL_HEIGHT 152

ret_code_t paper_init(void);
void paper_uninit(void);

void paper_pixel_draw(uint16_t x, uint16_t y, uint32_t color);
void paper_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

void paper_display(void);

void paper_rotation_set(nrf_lcd_rotation_t rotation);
void paper_display_invert(bool invert);

void paper_setColor(uint8_t color);

void  paper_drawBitmapBM(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

void paper_set_BWMode();
void paper_set_RMode();

void paper_print(nrf_lcd_t const * p_instance, uint8_t x, uint8_t y, char *text, uint8_t color);

#endif
