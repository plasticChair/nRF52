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
  screen.scDate.Y    = iconPosX[0]+0          ;  screen.scDate.X    = 30;
  screen.scTime.Y    = iconPosX[0]+0 +20      ;  screen.scTime.X    = 40;
  screen.humid.Y     = iconPosX[1] + 10       ;  screen.humid.X     = 33;
  screen.curTemp.Y   = iconPosX[2] + 10       ;  screen.curTemp.X   = 25;
  screen.tempH.Y     = iconPosX[2] + 0        ;  screen.tempH.X     = 80;
  screen.tempL.Y     = iconPosX[2] +20        ;  screen.tempL.X     = 80;
  screen.comp.Y      = iconPosX[3]            ;  screen.comp.X      = 40;
  screen.avgSpd.Y    = iconPosX[3]+textSize+5   ;  screen.avgSpd.X    = 40;
  
  // Column 2
  screen.curAlt.Y    = iconPosX[0]     ; screen.curAlt.X    = iconPosY[1] + 35;
  screen.altGain.Y   = iconPosX[1] - 10; screen.altGain.X   = iconPosY[1] + 35;
  screen.altLow.Y    = iconPosX[1] + 10; screen.altLow.X    = iconPosY[1] + 30;
  screen.distance.Y  = iconPosX[2]     ; screen.distance.X  = iconPosY[1]+ 33;
  screen.tripTime.Y  = iconPosX[2] + 30; screen.tripTime.X  = iconPosY[1] + 30;                                                     
  screen.battV.Y     = iconPosX[3] + 05; screen.battV.X     = iconPosY[1] + 40;

  //Column 1
  screen.iconDT.X    = iconPosX[0]     ; screen.iconDT.Y    = iconPosY[0];
  screen.iconHum.X   = iconPosX[1]     ; screen.iconHum.Y   = iconPosY[0];
  screen.iconTemp.X  = iconPosX[2]     ; screen.iconTemp.Y  = iconPosY[0];
  screen.iconTempH.X = iconPosX[2]-0   ; screen.iconTempH.Y = iconPosY[0]+60;
  screen.iconTempL.X = iconPosX[2]+16  ; screen.iconTempL.Y = iconPosY[0]+60;
  screen.iconComp.X  = iconPosX[3]     ; screen.iconComp.Y  = iconPosY[0]   ;
  screen.iconSpd.X   = iconPosX[3]     ; screen.iconSpd.Y   = iconPosY[0]   ;
                     
   // Column 2       
  screen.iconAlt.X   = iconPosX[0]-3 ; screen.iconAlt.Y   = iconPosY[1];
  screen.iconDis.X   = iconPosX[2]-5 ; screen.iconDis.Y   = iconPosY[1];
  screen.iconAltH.X  = iconPosX[0]+30; screen.iconAltH.Y  = iconPosY[1]+10;
  screen.iconAltL.X  = iconPosX[0]+50; screen.iconAltL.Y  = iconPosY[1]+10;
  screen.iconBatt.X  = iconPosX[3]   ; screen.iconBatt.Y  = iconPosY[1];

  //Status bar
  screen.iconStatusBar1.X = yScreenSize   ; screen.iconStatusBar1.Y = 0   ;
  screen.iconStatusBar2.X = yScreenSize   ; screen.iconStatusBar2.Y = 40  ;
  screen.iconStatusBar3.X = yScreenSize   ; screen.iconStatusBar3.Y = 80  ;
  screen.iconStatusBar4.X = yScreenSize   ; screen.iconStatusBar4.Y = 118 ;
  screen.iconStatusBar5.X = yScreenSize   ; screen.iconStatusBar5.Y = 152 ;
}



void screenSetStatus( bool statusIN, int statusNum)
{
  uint8_t tabSize = 40;
  uint8_t vertStart = tabSize*statusNum;
  uint8_t textOffset = 3;

  uint8_t tab_text[] = "GPS";
  uint8_t statusTxtBoarderOffset, statusTxtNewLineOffset;
  
  if (!statusIN)  {
      // Draw Black background
      // paper_drawBitmapBM(gImage_statusTabInactive      , 0, screen.iconStatusBar1.X , 25     , statusBarSize     , COLOR_WHITE);
      // display.setTextColor(GxEPD_BLACK);   
      paper_rect_draw(tabSize*(statusNum), yScreenSize,  tabSize, 30, COLOR_WHITE);
      paper_rect_draw(tabSize*(statusNum)      , yScreenSize, screenSize-tabSize*(statusNum),  3, COLOR_BLACK);
      paper_rect_draw(tabSize*(statusNum+1) - 2, yScreenSize,  2, 30, COLOR_BLACK);   
      paper_rect_draw(tabSize*(statusNum+1) - 3, yScreenSize  , 1, 8, COLOR_BLACK);
      paper_rect_draw(tabSize*(statusNum+1) - 7, yScreenSize+3, 5, 1, COLOR_BLACK);
      paper_pixel_draw(tabSize*(statusNum+1)-5 , yScreenSize+4,       COLOR_BLACK);
      paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+4,       COLOR_BLACK);
      paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+5,       COLOR_BLACK);
  }
  else {
    // Draw background behind red tab
    paper_rect_draw(tabSize*(statusNum), yScreenSize,  tabSize, 30, COLOR_RED);
    paper_rect_draw(tabSize*(statusNum)      , yScreenSize, screenSize-tabSize*(statusNum),  3, COLOR_WHITE);
    paper_rect_draw(tabSize*(statusNum+1) - 2, yScreenSize,  2, 30, COLOR_REDINV);
    paper_rect_draw(tabSize*(statusNum+1) - 3, yScreenSize  , 1, 8, COLOR_REDINV);
    paper_rect_draw(tabSize*(statusNum+1) - 7, yScreenSize+3, 5, 1, COLOR_REDINV);
    paper_pixel_draw(tabSize*(statusNum+1)-5 , yScreenSize+4,       COLOR_REDINV);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+4,       COLOR_REDINV);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+5,       COLOR_REDINV);
    paper_rect_draw(tabSize*(statusNum)      , yScreenSize, screenSize-tabSize*(statusNum),  3, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 2, yScreenSize,  2, 30, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 3, yScreenSize  , 1, 8, COLOR_BLACK);
    paper_rect_draw(tabSize*(statusNum+1) - 7, yScreenSize+3, 5, 1, COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-5 , yScreenSize+4,       COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+4,       COLOR_BLACK);
    paper_pixel_draw(tabSize*(statusNum+1)-4 , yScreenSize+5,       COLOR_BLACK);
  }

  statusTxtBoarderOffset = 10;
  statusTxtNewLineOffset = statusTxtBoarderOffset +10;

  if(statusNum == 0)  {
    if(statusIN){
     paper_print(&lcd, tabSize*statusNum + textOffset, yScreenSize + statusTxtBoarderOffset, "  GPS", COLOR_WHITE, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "  ACQ", COLOR_WHITE, TEXT_SMALL);
    }
    else {
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  GPS", COLOR_BLACK, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "   ACQ", COLOR_BLACK, TEXT_SMALL);
    }
  }
  else if(statusNum == 1)
  {
     if(statusIN){
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  GPS", COLOR_WHITE, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "TRACK", COLOR_WHITE, TEXT_SMALL);
    }
    else{
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  GPS", COLOR_BLACK, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "TRACK", COLOR_BLACK, TEXT_SMALL);
    }
  }
  
  else if(statusNum == 2)
  {
     if(statusIN){
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "  GPS", COLOR_WHITE, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "   ON", COLOR_WHITE, TEXT_SMALL);
    }
    else{
     paper_print(&lcd, 40*statusNum, yScreenSize + statusTxtBoarderOffset, "  GPS", COLOR_BLACK, TEXT_SMALL);
     paper_print(&lcd, 40*statusNum, yScreenSize + statusTxtBoarderOffset+7, " OFF", COLOR_BLACK, TEXT_SMALL);
    }
  }    
  else if(statusNum == 3)
  {
     if(statusIN){
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "   BT", COLOR_WHITE, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "   ON", COLOR_WHITE, TEXT_SMALL);
    }
    else{
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "   BT", COLOR_BLACK, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "   OFF", COLOR_BLACK, TEXT_SMALL);

    }
  }
  else if(statusNum == 4)
  {
     if(statusIN){
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "   SD", COLOR_WHITE, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "WRITE", COLOR_WHITE, TEXT_SMALL);
    }
    else{
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset, "   SD", COLOR_BLACK, TEXT_SMALL);
     paper_print(&lcd, tabSize*statusNum, yScreenSize + statusTxtBoarderOffset+7, "WRITE", COLOR_BLACK, TEXT_SMALL);
  }
}
}



void screenSetDate(char* dateIN)
{
   paper_print(&lcd,  screen.scDate.X,  screen.scDate.Y, dateIN, COLOR_RED, TEXT_LARGE);
}

void screenSetTime(char* timeIN)
{
  paper_print(&lcd,  screen.scTime.X,  screen.scTime.Y, timeIN, COLOR_RED, TEXT_LARGE);
}


void screenSetHumidity(float humidityIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.1f\0",humidityIN);
    paper_print(&lcd,  screen.humid.X,  screen.humid.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.humid.X + 40,  screen.humid.Y, "%", COLOR_BLACK, TEXT_LARGE);
}

void screenSetTemp(float tempIN, float tempHIN, float tempLIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.1f\0",tempIN);
    paper_print(&lcd,  screen.curTemp.X,  screen.curTemp.Y, buffer, COLOR_RED, TEXT_LARGE);
    nrf_gfx_circle_t circ_1 = NRF_GFX_CIRCLE(screen.curTemp.X+29, screen.curTemp.Y-1, 3);
    nrf_gfx_circle_draw(&lcd, &circ_1, 0x1, false);
    paper_print(&lcd,  screen.curTemp.X+30,  screen.curTemp.Y, "C", COLOR_BLACK, TEXT_LARGE);

    buffer[0] =  0;
    sprintf(buffer,"%2.0f\0",tempHIN);
    //display.drawCircle(screen.curTemp.Y+35, screen.curTemp.X-13,2,GxEPD_BLACK);
    paper_print(&lcd,  screen.tempH.X,  screen.tempH.Y, buffer, COLOR_RED, TEXT_LARGE);

    buffer[0] = 0;
    sprintf(buffer,"%2.0f\0",tempLIN);

    paper_print(&lcd,  screen.tempL.X,  screen.tempL.Y, buffer, COLOR_RED, TEXT_LARGE);
  
}

void screenSetCompass(float directionIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.0f\0",directionIN);
    paper_print(&lcd,  screen.comp.X,  screen.comp.Y, buffer, COLOR_RED, TEXT_LARGE);
    //paper_print(&lcd,  screen.comp.X+6,  screen.comp.Y, (char *)((char)223), COLOR_BLACK, TEXT_LARGE);
     nrf_gfx_circle_t circ_1 = NRF_GFX_CIRCLE(screen.comp.X+27, screen.comp.Y+1, 3);
    nrf_gfx_circle_draw(&lcd, &circ_1, 0x1, false);
}

void screenSetSpeed(float speedIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.1f\0",speedIN);
    paper_print(&lcd,  screen.avgSpd.X,  screen.avgSpd.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.avgSpd.X+25,  screen.avgSpd.Y, "mph", COLOR_BLACK, TEXT_LARGE);
}

void screenSetAlt(float altIN, float altHIN, float altLIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.0f\0",altIN);
    paper_print(&lcd,  screen.curAlt.X,  screen.curAlt.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.curAlt.X+ 35,screen.curAlt.Y, "ft", COLOR_BLACK, TEXT_LARGE);
    buffer[0] = 0;
    sprintf(buffer,"%2.0f\0",altHIN);
    paper_print(&lcd,  screen.altGain.X,  screen.altGain.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.altGain.X+35,  screen.altGain.Y, "ft", COLOR_BLACK, TEXT_LARGE);
    buffer[0] = 0;
    sprintf(buffer,"%2.0f\0",altLIN);
    paper_print(&lcd,  screen.altLow.X,  screen.altLow.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.altLow.X+39,  screen.altLow.Y, "ft", COLOR_BLACK, TEXT_LARGE);

}

void screenSetDistance(float distanceIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.2f\0",distanceIN);
    paper_print(&lcd,  screen.distance.X,  screen.distance.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.distance.X+40,  screen.distance.Y, "mi", COLOR_BLACK, TEXT_LARGE);
    paper_print(&lcd,  screen.distance.X,  screen.distance.Y+18, "002:15", COLOR_RED, TEXT_LARGE);
}

void screenSetBattery(float batteryIN)
{
    char buffer[10]; 
    sprintf(buffer,"%2.2f\0",batteryIN);
    paper_print(&lcd,  screen.battV.X,  screen.battV.Y, buffer, COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.battV.X+30l,  screen.battV.Y, "V", COLOR_BLACK, TEXT_LARGE);

    paper_print(&lcd,  screen.battV.X - 40,  screen.battV.Y+20, "01  14   53", COLOR_RED, TEXT_LARGE);
    paper_print(&lcd,  screen.battV.X-40,  screen.battV.Y+20, "   D   H    M", COLOR_BLACK, TEXT_LARGE);
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
    screenInit();
    nrf_gfx_screen_fill(&lcd, COLOR_WHITE);

    //Horz
    paper_rect_draw(0,rowHeight*2, screenSize/2, lineWidth, COLOR_BLACK);
    paper_rect_draw(screenSize/2,rowHeight*2-3, screenSize-screenSize/2, lineWidth, COLOR_BLACK);
    paper_rect_draw(0, rowHeight*3, screenSize, lineWidth, COLOR_BLACK);
    paper_rect_draw(0, screenSize - statusBarSize, screenSize, statusBarSize, COLOR_BLACK);
    paper_rect_draw(0,rowHeight, screenSize/2, lineWidth, COLOR_BLACK);


    //Vertical line
    paper_rect_draw(screenSize/numCols, 0, lineWidth, screenSize-statusBarSize, COLOR_BLACK);

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
    screenSetStatus( false, STATUS_BT);
    screenSetStatus( true, STATUS_SD);

    screenSetDistance(15.6);
    screenSetAlt(1238, 456,-187);
    screenSetSpeed(2.3);
    screenSetCompass(254.8);
    screenSetTemp(25.3, 32.2, 15.2);
    screenSetHumidity(38.2);
    screenSetTime("2:24");
    screenSetDate("04/13/18");
    screenSetBattery(3.84);

    nrf_gfx_display(&lcd);


   while (true)
      if(!NRF_LOG_PROCESS())
        __WFE();
        
  
}

/** @} */
