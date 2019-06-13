#ifndef PAPER_H
#define PAPER_H

#include "app_error.h"
#include "nrf_lcd.h"
#include "nrf_gfx.h"

// Display resolution
#define EPD_WIDTH       200
#define EPD_HEIGHT      200

// EPD1IN54B commands
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_COMMAND                  0x40
#define TEMPERATURE_SENSOR_CALIBRATION              0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define TCON_RESOLUTION                             0x61
#define SOURCE_AND_GATE_START_SETTING               0x62
#define GET_STATUS                                  0x71
#define AUTO_MEASURE_VCOM                           0x80
#define VCOM_VALUE                                  0x81
#define VCM_DC_SETTING_REGISTER                     0x82
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAM                              0xA1
#define READ_OTP_DATA                               0xA2


extern const nrf_gfx_font_desc_t georgia_6ptFontInfo;
extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;
extern const nrf_gfx_font_desc_t microsoftYaHei_6ptFontInfo; ;
extern const nrf_gfx_font_desc_t microsoftYaHei_10ptFontInfo; ;
static const nrf_gfx_font_desc_t * p_font = &georgia_6ptFontInfo;
static const nrf_gfx_font_desc_t * p_font_8 = &orkney_8ptFontInfo;
static const nrf_gfx_font_desc_t * p_font_6b = &microsoftYaHei_6ptFontInfo;
static const nrf_gfx_font_desc_t * p_font_10b = &microsoftYaHei_10ptFontInfo;

#define PAPER_PIXEL_WIDTH  200
#define PAPER_PIXEL_HEIGHT 200
#define TEXT_SMALL 0
#define TEXT_LARGE 2

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

void paper_print(nrf_lcd_t const * p_instance, uint8_t x, uint8_t y, char *text, uint8_t color, uint8_t size);
void paper_waitBusy();
#endif
