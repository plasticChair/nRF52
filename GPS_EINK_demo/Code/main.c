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



// Waveshare ePaper

//#include "nrf_gfx.h"
#include "waveshare_epd.h"
#include "ImageData.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define DEVICE_NAME                     "Waveshare Demo"   

static uint8_t disptxt_buffer[16] = "Hello Epaper\0\0\0\0";
static uint8_t epaper_pending;

extern const nrf_lcd_t nrf_lcd_wsepd154;
static const nrf_lcd_t * p_lcd = &nrf_lcd_wsepd154;
//extern const nrf_lcd_t * p_lcd;

extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;
static const nrf_gfx_font_desc_t * p_font = &orkney_8ptFontInfo;



static void epaper_demo_text(void)
{
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_270);
    nrf_gfx_point_t text_start = NRF_GFX_POINT(15, 70);
    nrf_gfx_screen_fill(p_lcd, 0xff);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start, 0x00, (const char *)disptxt_buffer, p_font, true));
    nrf_gfx_display(p_lcd);
    nrf_gfx_uninit(p_lcd);
}

/**@brief Draws geometric objects and text on epaper display
 *
 * @details Translation of Waveshare epaper demo code to run on nRF52
 */
static void epaper_demo_draw(void)
{
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_270);
    nrf_gfx_point_t text_start = NRF_GFX_POINT(15, 70);
    nrf_gfx_screen_fill(p_lcd, 0x00);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start, 0xff, DEVICE_NAME, p_font, true));
    nrf_gfx_circle_t circ_1 = NRF_GFX_CIRCLE(25, 25, 10);
    nrf_gfx_circle_draw(p_lcd, &circ_1, 0xff, false);
    circ_1.x = 125;
    nrf_gfx_circle_draw(p_lcd, &circ_1, 0xff, true);
    nrf_gfx_display(p_lcd);
    nrf_gfx_uninit(p_lcd);
}

static void epaper_demo_imarray(void)
{
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    wsepd154_draw_monobmp((const uint8_t *)gImage_1in54);
    nrf_gfx_display(p_lcd);
    nrf_gfx_uninit(p_lcd);
}

/**@brief Function for the Power Manager.
 */
static void power_manage(void)
{
    ret_code_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
    switch(epaper_pending)
    {
        case 1:
            NRF_LOG_INFO("Starting to display text\r\n");
            epaper_demo_text();
            epaper_pending = 0;
            break;
        case 2:
            NRF_LOG_INFO("Starting to draw demo\r\n");
            epaper_demo_draw();
            epaper_pending = 0;
            break;
        case 3:
            NRF_LOG_INFO("Starting to draw image\r\n");
            epaper_demo_imarray();
            epaper_pending = 0;
        default:
            break;
    }
}

static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}



int main(void)
{
    ret_code_t ret;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
    NRF_LOG_INFO("GPS EINK Example\r\n");
    NRF_LOG_FLUSH();




  NRF_LOG_INFO("paper example started");
  NRF_LOG_FLUSH();

 epaper_demo_draw();
epaper_demo_imarray();
epaper_demo_text();
  while (true)
    if(!NRF_LOG_PROCESS())
      __WFE();
}


/** @} */
