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

#include "icons/ico_l1.h"
#include "icons/ico_l2.h"
#include "icons/battery.h"
#include "icons/cal.h"
#include "icons/clock.h"
#include "icons/comp.h"
#include "icons/distance.h"
#include "icons/up.h"
#include "icons/down.h"
#include "icons/humidity.h"
#include "icons/mountains.h"
#include "icons/temp.h"
#include "icons/statusTabInactive.h"
#include "icons/statusTabActive.h"

#include "main.h"

static uint8_t disptxt_buffer[] = "Hello Epaper \r Sup player h8rs\0\0\0\0";
#define DEVICE_NAME                     "Waveshare Demo"   



struct screenField screen;


static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}



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

  int iconPosY[2] = {1, (rowWidth + iconEdgeSpace)};
  int iconPosX[4] = {iconEdgeSpace, (xIconSpace+ iconEdgeSpace), (xIconSpace*2 + iconEdgeSpace), (xIconSpace*3 + iconEdgeSpace)};


void screenInit()
{
//      <-- Y -->
//   ^   ----------
//   |   |        |
//   X   |        |
//   |   |        |
//   v   |        |
//       ----------
int textSize = 10;
  //Column 1
  screen.scDate.X    = iconPosX[0]+textSize    ;  screen.scDate.Y    = 28;
  screen.scTime.X    = iconPosX[0]+textSize +20;  screen.scTime.Y    = 35;
  screen.humid.X     = iconPosX[1] + 20       ;  screen.humid.Y     = 30;
  screen.curTemp.X   = iconPosX[2] + 20       ;  screen.curTemp.Y   = 20;
  screen.tempH.X     = iconPosX[2] +10        ;  screen.tempH.Y     = 80;
  screen.tempL.X     = iconPosX[2] +30        ;  screen.tempL.Y     = 80;
  screen.comp.X      = iconPosX[3] +textSize+2;  screen.comp.Y      = 35;
  screen.avgSpd.X    = iconPosX[3]+textSize+20;  screen.avgSpd.Y    = 35;
  

  // Column 2
  screen.curAlt.X    = iconPosX[0] + 20; screen.curAlt.Y    = iconPosY[1] + 25;
  screen.altGain.X   = iconPosX[1]     ; screen.altGain.Y   = iconPosY[1] + 35;
  screen.altLow.X    = iconPosX[1] + 20; screen.altLow.Y    = iconPosY[1] + 30;
  screen.distance.X  = iconPosX[2] + 10; screen.distance.Y  = iconPosY[1] + 30;
  screen.tripTime.X  = iconPosX[2] + 30; screen.tripTime.Y  = iconPosY[1] + 30;                                                     
  screen.battV.X     = iconPosX[3] + 15; screen.battV.Y     = iconPosY[1] + 30;

  //Column 1
  screen.iconDT.X    = iconPosX[0]-5     ; screen.iconDT.Y    = iconPosY[0];
  screen.iconHum.X   = iconPosX[1]     ; screen.iconHum.Y   = iconPosY[0];
  screen.iconTemp.X  = iconPosX[2]     ; screen.iconTemp.Y  = iconPosY[0];
  screen.iconTempH.X = iconPosX[2]-4   ; screen.iconTempH.Y = iconPosY[0]+43;
  screen.iconTempL.X = iconPosX[2]+12  ; screen.iconTempL.Y = iconPosY[0]+43;
  screen.iconComp.X  = iconPosX[3]     ; screen.iconComp.Y  = iconPosY[0]   ;
  screen.iconSpd.X   = iconPosX[3]     ; screen.iconSpd.Y   = iconPosY[0]   ;
                     
   // Column 2       
  screen.iconAlt.X   = iconPosX[0]-3 ; screen.iconAlt.Y   = iconPosY[1];
  screen.iconDis.X   = iconPosX[2]-5 ; screen.iconDis.Y   = iconPosY[1];
  screen.iconAltH.X  = iconPosX[0]+20; screen.iconAltH.Y  = iconPosY[1]+10;
  screen.iconAltL.X  = iconPosX[0]+35; screen.iconAltL.Y  = iconPosY[1]+10;
  screen.iconBatt.X  = iconPosX[3]   ; screen.iconBatt.Y  = iconPosY[1];

  //Status bar
  screen.iconStatusBar1.X = yScreenSize   ; screen.iconStatusBar1.Y = 0   ;
  screen.iconStatusBar2.X = yScreenSize   ; screen.iconStatusBar2.Y = 40  ;
  screen.iconStatusBar3.X = yScreenSize   ; screen.iconStatusBar3.Y = 80  ;
  screen.iconStatusBar4.X = yScreenSize   ; screen.iconStatusBar4.Y = 118 ;
  screen.iconStatusBar5.X = yScreenSize   ; screen.iconStatusBar5.Y = 152 ;

  //screen.iconStatusBar5 = {yScreenSize   , 160   };
 
}



void screenSetStatus( bool statusIN, int statusNum)
{
  uint8_t tabSize = 30;
  uint8_t vertStart = tabSize*statusNum;
  uint8_t textOffset = 3;

  
  
  uint8_t tab_text[] = "GPS";
  //const GFXfont* f = &SansSerif_bold_8;
  uint8_t statusTxtBoarderOffset, statusTxtNewLineOffset;
  //display.setFont(f);
  
  //nrf_gfx_point_t text_start = NRF_GFX_POINT(10, 00);
 //  APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start, 0x1, (const char *)disptxt_buffer, p_font, true));

  
  
  if (!statusIN)  {
    // Draw Black background
   // paper_drawBitmapBM(gImage_statusTabInactive      , 0, screen.iconStatusBar1.X , 25     , statusBarSize     , COLOR_WHITE);
   // display.setTextColor(GxEPD_BLACK);   
   paper_rect_draw(tabSize*(statusNum), yScreenSize,  tabSize, 30, COLOR_WHITE);
   paper_rect_draw(tabSize*(statusNum)      , yScreenSize, screenSize-tabSize*(statusNum),  3, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 2, yScreenSize,  2, 30, COLOR_BLACK);
//      
    paper_rect_draw(tabSize*(statusNum+1) - 3, yScreenSize  , 1, 8, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 7, yScreenSize+3, 5, 1, COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-5 , yScreenSize+4,       COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+4,       COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+5,       COLOR_BLACK);


  }
  else {
    // Draw background behind red tab
    // paper_drawBitmapBM(gImage_statusTabActive      , 0, screen.iconStatusBar1.X , 25     , statusBarSize     , COLOR_RED);
    paper_rect_draw(tabSize*(statusNum), yScreenSize,  tabSize, 30, COLOR_RED);

  
    paper_rect_draw(tabSize*(statusNum)      , yScreenSize, screenSize-tabSize*(statusNum),  3, COLOR_REDINV);
    paper_rect_draw(tabSize*(statusNum+1) - 2, yScreenSize,  2, 30, COLOR_REDINV);
//      
    paper_rect_draw(tabSize*(statusNum+1) - 3, yScreenSize  , 1, 8, COLOR_REDINV);
    paper_rect_draw(tabSize*(statusNum+1) - 7, yScreenSize+3, 5, 1, COLOR_REDINV);
    paper_pixel_draw(tabSize*(statusNum+1)-5 , yScreenSize+4,       COLOR_REDINV);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+4,       COLOR_REDINV);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+5,       COLOR_REDINV);

       paper_rect_draw(tabSize*(statusNum)      , yScreenSize, screenSize-tabSize*(statusNum),  3, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 2, yScreenSize,  2, 30, COLOR_BLACK);
//      
    paper_rect_draw(tabSize*(statusNum+1) - 3, yScreenSize  , 1, 8, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 7, yScreenSize+3, 5, 1, COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-5 , yScreenSize+4,       COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+4,       COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+5,       COLOR_BLACK);
  }

  statusTxtBoarderOffset = 10;
  statusTxtNewLineOffset = statusTxtBoarderOffset +10;

  if(statusNum == 0)  {
    //display.setCursor(40*statusNum,  yScreenSize+15  );
    if(statusIN){
     paper_print(&lcd, tabSize*statusNum + textOffset, yScreenSize + statusTxtBoarderOffset, " GPS", COLOR_REDINV);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, " Abp", COLOR_REDINV);

    //   nrf_gfx_point_t text_start = NRF_GFX_POINT(tabSize*statusNum + textOffset,  yScreenSize + statusTxtBoarderOffset);

  //  APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start, COLOR_REDINV, (const char *)" GPS", p_font, true));
//nrf_gfx_point_t text_start2 = NRF_GFX_POINT(tabSize*statusNum + textOffset,  yScreenSize + statusTxtBoarderOffset);
//    APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start2, COLOR_REDINV, (const char *)" Acq", p_font, true));


    }
    else {
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, " GPS", COLOR_BLACK);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, " Abp", COLOR_BLACK);
    }
    
  }
  else if(statusNum == 1)
  {
     if(statusIN){
          paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, " GPS", COLOR_REDINV);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "Tq`bj", COLOR_REDINV);
    }
    else{

     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, " GPS", COLOR_BLACK);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "Tq`bj", COLOR_BLACK);
    }
  }
  
  else if(statusNum == 2)
  {
     if(statusIN){
                paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, " GPS", COLOR_REDINV);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "  ON", COLOR_REDINV);

       


//      display.setTextColor(COLOR_WHITE);  
//      display.setCursor(40*statusNum,  yScreenSize + statusTxtBoarderOffset  );
//      display.print("  GPS");
//      display.setCursor(40*statusNum,  yScreenSize + statusTxtNewLineOffset  );
//      display.print("  On");
    }
    else{
              paper_print(&lcd, 40*statusNum, yScreenSize + statusTxtBoarderOffset, " GPS", COLOR_BLACK);
     paper_print(&lcd, 40*statusNum, yScreenSize + statusTxtBoarderOffset+7, " Off", COLOR_BLACK);

       
//      display.setTextColor(COLOR_BLACK);  
//     display.setCursor(40*statusNum,  yScreenSize + statusTxtBoarderOffset  );
//     display.print("  GPS");
//     display.setCursor(40*statusNum,  yScreenSize + statusTxtNewLineOffset  );
//     display.print("   Off"); 
    }
  }    
  else if(statusNum == 3)
  {
     if(statusIN){
                   paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  BT", COLOR_REDINV);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "  ON", COLOR_REDINV);
//      display.setTextColor(COLOR_WHITE);  
//      display.setCursor(40*statusNum,  yScreenSize + statusTxtBoarderOffset  );
//      display.print("   BT");
//      display.setCursor(40*statusNum,  yScreenSize + statusTxtNewLineOffset  );
//      display.print("   On");
    }
    else{
                  paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  BT", COLOR_BLACK);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, " OFF", COLOR_BLACK);
//      display.setTextColor(COLOR_BLACK);  
//     display.setCursor(40*statusNum,  yScreenSize + statusTxtBoarderOffset  );
//     display.print("   BT");
//     display.setCursor(40*statusNum,  yScreenSize + statusTxtNewLineOffset  );
//     display.print("   Off"); 
    }

  }
  else if(statusNum == 4)
  {
     if(statusIN){
                  paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  SD", COLOR_REDINV);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "Wqhsd", COLOR_REDINV);
//      display.setTextColor(COLOR_WHITE);  
//      display.setCursor(40*statusNum,  yScreenSize + statusTxtBoarderOffset  );
//      display.print("   SD");
//      display.setCursor(40*statusNum,  yScreenSize + statusTxtNewLineOffset  );
//      display.print("   OFF");
    }
    else{
                      paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  SD", COLOR_BLACK);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "Wqhsd", COLOR_BLACK);
//     display.setTextColor(COLOR_BLACK);  
//     display.setCursor(40*statusNum,  yScreenSize + statusTxtBoarderOffset  );
//     display.print("   SD");
//     display.setCursor(40*statusNum,  yScreenSize + statusTxtNewLineOffset  );
//     display.print(" Write"); 
    }
  }
}

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
  //paper_set_BWMode();
  screenInit();
    nrf_gfx_screen_fill(&lcd, 0x0);

     //Vertical line
  paper_rect_draw(screenSize/numCols, 0, lineWidth, screenSize-statusBarSize, COLOR_BLACK);
  //Horz
  //void paper_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
  paper_rect_draw(0,rowHeight, screenSize/2, lineWidth, COLOR_BLACK);
  paper_rect_draw(0,rowHeight*2, screenSize/2, lineWidth, COLOR_BLACK);
  paper_rect_draw(screenSize/2,rowHeight*2-3, screenSize-screenSize/2, lineWidth, COLOR_BLACK);
  paper_rect_draw(0, rowHeight*3, screenSize, lineWidth, COLOR_BLACK);
//  paper_rect_draw(0, screenSize - statusBarSize, screenSize, statusBarSize, COLOR_BLACK);
  
 // nrf_gfx_display(&lcd);
  //nrf_delay_ms(1000);
  //paper_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
  // Draw Icons
  paper_drawBitmapBM(gImage_cal          , screen.iconDT.Y   , screen.iconDT.X   , iconSize     , iconSize     , COLOR_BLACK);
  paper_drawBitmapBM(gImage_humidity     , screen.iconHum.Y  , screen.iconHum.X  , iconSize     , iconSize     , COLOR_BLACK);
  paper_drawBitmapBM(gImage_temp         , screen.iconTemp.Y , screen.iconTemp.X , iconSize     , iconSize     , COLOR_BLACK);
  paper_drawBitmapBM(gImage_up           , screen.iconTempH.Y, screen.iconTempH.X, iconSizeSmall, iconSizeSmall, COLOR_BLACK);
  paper_drawBitmapBM(gImage_down         , screen.iconTempL.Y, screen.iconTempL.X, iconSizeSmall, iconSizeSmall, COLOR_BLACK);
  paper_drawBitmapBM(gImage_comp         , screen.iconComp.Y , screen.iconComp.X , iconSize     , iconSize     , COLOR_BLACK);
  paper_drawBitmapBM(gImage_mountains    , screen.iconAlt.Y  , screen.iconAlt.X  , iconSize     , iconSize     , COLOR_BLACK);
  paper_drawBitmapBM(gImage_distance     , screen.iconDis.Y  , screen.iconDis.X  , iconSize     , iconSize     , COLOR_BLACK);
  paper_drawBitmapBM(gImage_up           , screen.iconAltH.Y , screen.iconAltH.X , iconSizeSmall, iconSizeSmall, COLOR_BLACK);
  paper_drawBitmapBM(gImage_down         , screen.iconAltL.Y , screen.iconAltL.X , iconSizeSmall, iconSizeSmall, COLOR_BLACK);
  paper_drawBitmapBM(gImage_battery      , screen.iconBatt.Y , screen.iconBatt.X , iconSize     , iconSize     , COLOR_BLACK);
  
  
 paper_rect_draw(0, yScreenSize,  screenSize,2, COLOR_BLACK);
 screenSetStatus( true, STATUS_ACQ);
 screenSetStatus( false, STATUS_TRK);
 screenSetStatus( true, STATUS_GPS);
 screenSetStatus( true, STATUS_BT);
 screenSetStatus( false, STATUS_SD);

  nrf_gfx_display(&lcd);

//
//   display.fillRect(0, yScreenSize, screenSize, statusBarSize, GxEPD_BLACK);

//nrf_gfx_rotation_set(&lcd, NRF_LCD_ROTATE_90);
//  // draw a horizontal line
//  nrf_gfx_line_t hline = NRF_GFX_LINE(
//    0,
//    nrf_gfx_height_get(&lcd) / 3,
//    nrf_gfx_width_get(&lcd),
//    nrf_gfx_height_get(&lcd) / 3,
//    4
//  );
//  nrf_gfx_line_draw(&lcd, &hline, 2);
//
//  // draw a vertical line
//  nrf_gfx_line_t vline = NRF_GFX_LINE(
//    nrf_gfx_width_get(&lcd) / 2,
//    0,
//    nrf_gfx_width_get(&lcd) / 2,
//    nrf_gfx_height_get(&lcd),
//    4
//  );
//  nrf_gfx_line_draw(&lcd, &vline, 1);
//
// 
//  nrf_gfx_display(&lcd);
//
//  nrf_delay_ms(15000);
//
//    nrf_gfx_rotation_set(&lcd, NRF_LCD_ROTATE_90);
//    nrf_gfx_point_t text_start = NRF_GFX_POINT(10, 00);
//    nrf_gfx_screen_fill(&lcd, 0x0);
//    APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start, 0x1, (const char *)disptxt_buffer, p_font, true));
//  //  nrf_gfx_display(&lcd);
//    //nrf_gfx_uninit(&lcd);
//
//  nrf_gfx_display(&lcd);
//  nrf_delay_ms(15000);
//
//      //APP_ERROR_CHECK(nrf_gfx_init(&lcd));
//    nrf_gfx_rotation_set(&lcd, NRF_LCD_ROTATE_270);
//     nrf_gfx_screen_fill(&lcd, 0x00);
//    APP_ERROR_CHECK(nrf_gfx_print(&lcd, &text_start, 0xff, DEVICE_NAME, p_font, true));
//    nrf_gfx_circle_t circ_1 = NRF_GFX_CIRCLE(25, 25, 10);
//    nrf_gfx_circle_draw(&lcd, &circ_1, 0x1, false);
//    circ_1.x = 125;
//        nrf_gfx_circle_draw(&lcd, &circ_1, 0x2, true);
//    nrf_gfx_display(&lcd);
////    nrf_gfx_uninit(&lcd);


  while (true)
    if(!NRF_LOG_PROCESS())
      __WFE();
}


/** @} */
