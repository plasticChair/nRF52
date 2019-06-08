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
/** @file
 * @defgroup fatfs_example_main main.c
 * @{
 * @ingroup fatfs_example
 * @brief FATFS Example Application main file.
 *
 * This file contains the source code for a sample application using FAT filesystem and SD card library.
 *
 */

/* 

SD Card with Button example and timer
Records timer to sd card each button press.  Then reads it back


*/
#include "nrf.h"
#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"
#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "bsp.h"

#include "app_timer.h"
#include "nordic_common.h"

#include "nrf_drv_timer.h"

#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "GlobalTypes.h"

#define FILE_NAME   "NORDIC.TXT"
#define TEST_STRING "SD card example."

#define SDC_SCK_PIN     ARDUINO_13_PIN  ///< SDC serial clock (SCK) pin.  25
#define SDC_MOSI_PIN    ARDUINO_11_PIN  ///< SDC serial data in (DI) pin.  23
#define SDC_MISO_PIN    ARDUINO_12_PIN  ///< SDC serial data out (DO) pin.  24
#define SDC_CS_PIN      ARDUINO_10_PIN  ///< SDC chip select (CS) pin.     22


#define BUTTON_PREV_ID           0                           /**< Button used to switch the state. */
#define BUTTON_NEXT_ID           1                           /**< Button used to switch the state. */

static bsp_indication_t actual_state =  BSP_INDICATE_FIRST;         /**< Currently indicated state. */

static const char * indications_list[] = BSP_INDICATIONS_LIST;

const nrf_drv_timer_t TIMER_LED = NRF_DRV_TIMER_INSTANCE(0);

struct SD_Card_Struct Mstr_SDCard;

volatile bool sdFlag = false;
volatile int timercount = 0;
/**
 * @brief Handler for timer events.
 */
void timer_led_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    static uint32_t i;
    uint32_t led_to_invert = ((i++) % LEDS_NUMBER);

    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            bsp_board_led_invert(led_to_invert);
            break;

        default:
            //Do nothing.
            break;
    }
 //   uint32_t captured_value = current_nrf_drv_timer_capture(&TIMER_LED, 1);
 //   NRF_TIMER2->TASKS_CAPTURE[0] = 1;
 //   uint32_t count =  NRF_TIMER2->CC[0];
  //  NRF_TIMER2->TASKS_CLEAR = 1;
    //NRF_LOG_INFO("%d",(count));
}


/**
 * @brief  SDC block device definition
 * */
NRF_BLOCK_DEV_SDC_DEFINE(
        m_block_dev_sdc,
        NRF_BLOCK_DEV_SDC_CONFIG(
                SDC_SECTOR_SIZE,
                APP_SDCARD_CONFIG(SDC_MOSI_PIN, SDC_MISO_PIN, SDC_SCK_PIN, SDC_CS_PIN)
         ),
         NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "SDC", "1.00")
);

void mountSD()
{
 // Initialize FATFS disk I/O interface by providing the block device.
    static diskio_blkdev_t drives[] =
    {
            DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev), NULL)
    };

    diskio_blockdev_register(drives, ARRAY_SIZE(drives));

    NRF_LOG_INFO("Initializing disk 0 (SDC)...");
    for (uint32_t retries = 3; retries && Mstr_SDCard.disk_state; --retries)
    {
        Mstr_SDCard.disk_state = disk_initialize(0);
    }
    if (Mstr_SDCard.disk_state)
    {
        NRF_LOG_INFO("Disk initialization failed.");
        return;
    }

     NRF_LOG_INFO("Mounting volume...");
    Mstr_SDCard.ff_result = f_mount(&Mstr_SDCard.fs, "", 1);
    if (Mstr_SDCard.ff_result)
    {
        NRF_LOG_INFO("Mount failed.");
        return;
    }

}

void getSizeSD()
{
    uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_size;
    uint32_t capacity = m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_count / blocks_per_mb;
    NRF_LOG_INFO("Capacity: %d MB", capacity);

}

void listDirSD()
{

    NRF_LOG_INFO("\r\n Listing directory: /");
    Mstr_SDCard.ff_result = f_opendir(&Mstr_SDCard.dir, "/");
    if (Mstr_SDCard.ff_result)
    {
        NRF_LOG_INFO("Directory listing failed!");
        return;
    }

    do
    {
        Mstr_SDCard.ff_result = f_readdir(&Mstr_SDCard.dir, &Mstr_SDCard.fno);
        if (Mstr_SDCard.ff_result != FR_OK)
        {
            NRF_LOG_INFO("Directory read failed.");
            return;
        }

        if (Mstr_SDCard.fno.fname[0])
        {
            if (Mstr_SDCard.fno.fattrib & AM_DIR)
            {
                NRF_LOG_RAW_INFO("   <DIR>   %s",(uint32_t)Mstr_SDCard.fno.fname);
            }
            else
            {
                NRF_LOG_RAW_INFO("%9lu  %s", Mstr_SDCard.fno.fsize, (uint32_t)Mstr_SDCard.fno.fname);
            }
        }
    }
    while (Mstr_SDCard.fno.fname[0]);
    NRF_LOG_RAW_INFO("");
}


void openfileSD(char *filename, uint8_t mode)
{
    Mstr_SDCard.ff_result = f_open(&Mstr_SDCard.file, filename, mode);
    if (Mstr_SDCard.ff_result != FR_OK)
    {
        NRF_LOG_INFO("\rUnable to open or create file: %s .",*filename);
        return;
    }
    else {
     NRF_LOG_INFO("\rSuccessful open of file: %s .",filename);
     }
}

void writeFileSD(char *text)
{
    //NRF_LOG_INFO("Writing to file " FILE_NAME "...");
   

    Mstr_SDCard.ff_result = f_write(&Mstr_SDCard.file, text, strlen(text), (UINT *) &Mstr_SDCard.bytes_written);
    if (Mstr_SDCard.ff_result != FR_OK)
    {
        NRF_LOG_INFO("Write failed\r\n.");
    }
    else
    {
        NRF_LOG_INFO("%d bytes written.", Mstr_SDCard.bytes_written);
    }
}

void readFileSD(uint16_t numBytes)
{
 NRF_LOG_INFO(" File data: ");
  if (numBytes != 0){
      if (numBytes > 500) numBytes= 500;
     char tempBuff[500] = {0};

      Mstr_SDCard.ff_result = f_read(&Mstr_SDCard.file, tempBuff, numBytes, (UINT *) &Mstr_SDCard.bytes_read);
      if (Mstr_SDCard.ff_result != FR_OK)
      {
          NRF_LOG_INFO("read failed\r\n.");
      }
      else
      {
          NRF_LOG_INFO("%d bytes read.", Mstr_SDCard.bytes_read);
      }

       NRF_LOG_INFO("%s ", tempBuff);
  }
  else{
      char tempBuff[500] = {0};

      while (!f_eof(&Mstr_SDCard.file))
      {
        Mstr_SDCard.ff_result = f_read(&Mstr_SDCard.file, tempBuff, 500, (UINT *) &Mstr_SDCard.bytes_read);
        if (Mstr_SDCard.ff_result != FR_OK)
        {
            NRF_LOG_INFO("read failed\r\n.");
            return;
        }
         NRF_LOG_INFO("%s ", tempBuff);
      }
  }

} 

void closeFileSD()
{
  (void) f_close(&Mstr_SDCard.file);
}

/**
 * @brief Function for demonstrating FAFTS usage.
 */
static void fatfs_example()
{
    char filename[] = "Test2.txt";
    char string1[] = "testerdfjshdfjkhdfhsjdhfjkhdsfhhf784yfuihuifh4uifhi4uhfuihrfjkhsdkfhui4hfh4uihr1\n\0";
    char string2[] = "test3\n\0";

    Mstr_SDCard.disk_state = STA_NOINIT;
     mountSD();

    listDirSD();
    openfileSD(filename,FA_WRITE | FA_OPEN_APPEND);
    writeFileSD(string1);
    writeFileSD(string2);
    closeFileSD();

  //  openfileSD(filename, FA_READ);
  //  readFileSD(0);
   // closeFileSD();
    
    return;
}

/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    char filename[] = "timer.txt";
    char tempbuffer[100];
    uint32_t err_code;
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            if (actual_state != BSP_INDICATE_FIRST)
                actual_state--;
            else
                actual_state = BSP_INDICATE_LAST;
            break;

        case BSP_EVENT_KEY_1:

            if (actual_state != BSP_INDICATE_LAST)
                actual_state++;
            else
                actual_state = BSP_INDICATE_FIRST;
            break;

        default:
            return; // no implementation needed
    }
    err_code = bsp_indication_set(actual_state);
   NRF_LOG_INFO("%s", (uint32_t)indications_list[actual_state]);
    APP_ERROR_CHECK(err_code);

 NRF_TIMER2->TASKS_CAPTURE[0] = 1;
    uint32_t count =  NRF_TIMER2->CC[0];
    NRF_TIMER2->TASKS_CLEAR = 1;
    NRF_LOG_INFO("%d",(count));




    sdFlag = true;
    timercount = count;

}

void clock_initialization()
{
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }
}

void bsp_configuration()
{
    uint32_t err_code;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_evt_handler);
    APP_ERROR_CHECK(err_code);
}



/**
 * @brief Function for main application entry.
 */
int main(void)
{
uint32_t time_ms = 500; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("-------------------");

    clock_initialization();

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("BSP example started.");
    bsp_configuration();

    
    
    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    err_code = nrf_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    APP_ERROR_CHECK(err_code);
    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);

    nrf_drv_timer_extended_compare(
    &TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_enable(&TIMER_LED);

    // Manual method
    NRF_TIMER2->BITMODE = 03ul; // 0 = 16 bit, 1 = 8bit, 2 = 24bit, 3 = 32bit
    NRF_TIMER2->MODE = 0ul;  // 0 = timer, 1 = counter, 2 = low power counter
    NRF_TIMER2->TASKS_CLEAR = 1;
    NRF_TIMER2->PRESCALER = 9;  // 16MHz/2^9
    NRF_TIMER2->INTENSET = 1ul << 16;
    NRF_TIMER2->TASKS_START =1;
    NRF_TIMER2->TASKS_CAPTURE[0] =1;




 //   bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);


    NRF_LOG_INFO("FATFS example started.");

    fatfs_example();

nrf_delay_ms(55);
    fatfs_example();

    char tempBuff[100];

    while (true)
    {

        if (sdFlag)
        {
        sdFlag = false;
            openfileSD("timer.txt",FA_WRITE | FA_OPEN_APPEND);

            sprintf(tempBuff,"%d\r",timercount);
            writeFileSD(tempBuff);
            closeFileSD();

            openfileSD("timer.txt", FA_READ);
            readFileSD(0);
            closeFileSD();
    
        }
        NRF_LOG_FLUSH();
        __SEV();
        __WFE();
        __WFE();
    }
}

/** @} */
