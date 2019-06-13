/**
 * Copyright (c) 2016 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// #include "main.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_lcd.h"
#include "nrf_gfx.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_serial.h"

#include "GlobalTypes.h"
#include "paper.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"



static uint8_t disptxt_buffer[] = "Hello Epaper \r Sup player h8rs\0\0\0\0";
#define DEVICE_NAME                     "Waveshare Demo"   

extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;
static const nrf_gfx_font_desc_t * p_font = &orkney_8ptFontInfo;

static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}

extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;

static lcd_cb_t lcd_cb = {
 .state    = NRFX_DRV_STATE_UNINITIALIZED,
 .height   = PAPER_PIXEL_HEIGHT,
 .width    = PAPER_PIXEL_WIDTH,
 .rotation = NRF_LCD_ROTATE_270
};

static const nrf_lcd_t lcd = {
  .lcd_init = paper_init,
  .lcd_uninit = paper_uninit,
  .lcd_pixel_draw = paper_pixel_draw,
  .lcd_rect_draw = paper_rect_draw,
  .lcd_display = paper_display,
  .lcd_rotation_set = paper_rotation_set,
  .lcd_display_invert = paper_display_invert,
  .p_lcd_cb = &lcd_cb
};

int main(void)
{
    ret_code_t ret;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
    NRF_LOG_INFO("GPS EINK Example\r\n");
    NRF_LOG_FLUSH();


 APP_ERROR_CHECK(nrf_gfx_init(&lcd));

  NRF_LOG_INFO("paper example started");
  NRF_LOG_FLUSH();
    nrf_gfx_screen_fill(&lcd, 0x0);
nrf_gfx_rotation_set(&lcd, NRF_LCD_ROTATE_90);
  // draw a horizontal line
  nrf_gfx_line_t hline = NRF_GFX_LINE(
    0,
    nrf_gfx_height_get(&lcd) / 3,
    nrf_gfx_width_get(&lcd),
    nrf_gfx_height_get(&lcd) / 3,
    4
  );
  nrf_gfx_line_draw(&lcd, &hline, 2);

  // draw a vertical line
  nrf_gfx_line_t vline = NRF_GFX_LINE(
    nrf_gfx_width_get(&lcd) / 2,
    0,
    nrf_gfx_width_get(&lcd) / 2,
    nrf_gfx_height_get(&lcd),
    4
  );
  nrf_gfx_line_draw(&lcd, &vline, 1);

 
  nrf_gfx_display(&lcd);

  nrf_delay_ms(15000);

    nrf_gfx_rotation_set(&lcd, NRF_LCD_ROTATE_90);
    nrf_gfx_point_t text_start = NRF_GFX_POINT(10, 00);
    nrf_gfx_screen_fill(&lcd, 0x0);
    APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start, 0x1, (const char *)disptxt_buffer, p_font, true));
  //  nrf_gfx_display(&lcd);
    //nrf_gfx_uninit(&lcd);

  nrf_gfx_display(&lcd);
  nrf_delay_ms(15000);

      //APP_ERROR_CHECK(nrf_gfx_init(&lcd));
    nrf_gfx_rotation_set(&lcd, NRF_LCD_ROTATE_270);
     nrf_gfx_screen_fill(&lcd, 0x00);
    APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start, 0xff, DEVICE_NAME, p_font, true));
    nrf_gfx_circle_t circ_1 = NRF_GFX_CIRCLE(25, 25, 10);
    nrf_gfx_circle_draw(&lcd, &circ_1, 0x1, false);
    circ_1.x = 125;
        nrf_gfx_circle_draw(&lcd, &circ_1, 0x2, true);
    nrf_gfx_display(&lcd);
//    nrf_gfx_uninit(&lcd);


  while (true)
    if(!NRF_LOG_PROCESS())
      __WFE();
}


/** @} */
