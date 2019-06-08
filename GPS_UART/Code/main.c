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

#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_power.h"
#include "nrf_serial.h"
#include "app_timer.h"
#include "minmea.h"
#include "GlobalTypes.h"
#include "bsp.h"
#include "bsp_nfc.h"

#include "app_error.h"
#include "app_util.h"
#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"



#define OP_QUEUES_SIZE          3
#define APP_TIMER_PRESCALER     NRF_SERIAL_APP_TIMER_PRESCALER
#define MAX_BUFF_SIZE 250
#define INDENT_SPACES "  "
#define GPS_Debug_Print 0


static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}

NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config,
                      RX_PIN_NUMBER, TX_PIN_NUMBER,
                      RTS_PIN_NUMBER, CTS_PIN_NUMBER,
                      NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                      NRF_UART_BAUDRATE_115200,
                      UART_DEFAULT_CONFIG_IRQ_PRIORITY);


#define SERIAL_FIFO_TX_SIZE 1000
#define SERIAL_FIFO_RX_SIZE 1000

NRF_SERIAL_QUEUES_DEF(serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);


#define SERIAL_BUFF_TX_SIZE 100
#define SERIAL_BUFF_RX_SIZE 100

NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_IRQ,
                      &serial_queues, &serial_buffs, NULL, sleep_handler);


NRF_SERIAL_UART_DEF(serial_uart, 0);

struct NMEA_GPS_Struct MstrGPSData;

void gpsParse(char *line);
float position2float(int32_t position);
void printGPS();
bool gpsGetLine();
void resetGPSLine();

char tempStr[] = "-----------\r";

int main(void)
{
    ret_code_t ret;

    MstrGPSData.GPS_Done = false;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
    NRF_LOG_INFO("GPS UART Example\r\n");
    NRF_LOG_FLUSH();

    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);
    ret = nrf_drv_power_init(NULL);
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);
    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    // Initialize LEDs and buttons.
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);

    ret = nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
    APP_ERROR_CHECK(ret);

    static char tx_message[] = "Hello nrf_serial!\n\r";

    ret = nrf_serial_write(&serial_uart,
                           tx_message,
                           strlen(tx_message),
                           NULL,
                           NRF_SERIAL_MAX_TIMEOUT);
    APP_ERROR_CHECK(ret);

 
    NRF_LOG_INFO("Starting Program");
    NRF_LOG_FLUSH();
  
    resetGPSLine();

    while (true)
    {

          if (gpsGetLine()){


            (void)nrf_serial_write(&serial_uart, &MstrGPSData.line, MstrGPSData.lineSize, NULL, 50);
            (void)nrf_serial_flush(&serial_uart, 0);
         
           //NRF_LOG_INFO("%s",(uint32_t)buff2);
            gpsParse(MstrGPSData.line);
            resetGPSLine();
          }
      

        if (MstrGPSData.GPS_Done){
          MstrGPSData.GPS_Done = false;
          (void)nrf_serial_write(&serial_uart, "\r\r\r", 3, NULL, 50);
          (void)nrf_serial_flush(&serial_uart, 0);
 
        //  printGPS();
        //  NRF_LOG_INFO("------------------------");
       //   NRF_LOG_FLUSH();


        }

           //  (void)nrf_serial_write(&serial_uart, &tempStr,sizeof(tempStr), NULL, 50);
           //(void)nrf_serial_flush(&serial_uart, 0);

           sleep_handler();

    }
}

void resetGPSLine()
{
    MstrGPSData.lineSize = 0;
    MstrGPSData.line[0] = 0;
    MstrGPSData.linePtr  =  MstrGPSData.line;
}

bool gpsGetLine(char *line)
{
    static ret_code_t ret;   
    static char c;

    ret = nrf_serial_read(&serial_uart, &c, sizeof(c), NULL, 50);
    if (ret == NRF_SUCCESS)
    {
        *MstrGPSData.linePtr = c;
         MstrGPSData.linePtr++;
         MstrGPSData.lineSize++;
    #if 0
       *buffPtr = c;
        buffPtr++;
        buffIdx++;

        if (buffIdx >= MAX_BUFF_SIZE-1){
            *buffPtr = 0;
            (void)nrf_serial_write(&serial_uart, &buff, sizeof(buff), NULL, 0);
           (void)nrf_serial_flush(&serial_uart, 0);
            buff[0] = 0;
            buffPtr  = buff;
            buffIdx = 0;
        }
      #endif

        // If got new line break
        if (c == 0x0a){
          *MstrGPSData.linePtr = 0;
          c = 0;
          return true;
        }
    }
    else{
 //        ret = nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
  //      APP_ERROR_CHECK(ret);
        return false;
    }
  return false;
}


void gpsParse(char *line)
{
//while (fgets(line, sizeof(line), stdin) != NULL) {
        //GPS_Debug_Print
        #if 0
            printf("%s\r", line);
        #endif
        switch (minmea_sentence_id(line, false)) {
            case MINMEA_SENTENCE_RMC: {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, line)) {
                    printf(INDENT_SPACES "$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
                            frame.latitude.value, frame.latitude.scale,
                            frame.longitude.value, frame.longitude.scale,
                            frame.speed.value, frame.speed.scale);
                    printf(INDENT_SPACES "$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
                            minmea_rescale(&frame.latitude, 1000),
                            minmea_rescale(&frame.longitude, 1000),
                            minmea_rescale(&frame.speed, 1000));
                    printf(INDENT_SPACES "$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
                            minmea_tocoord(&frame.latitude),
                            minmea_tocoord(&frame.longitude),
                            minmea_tofloat(&frame.speed));
                }
                else {
                    printf(INDENT_SPACES "$xxRMC sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GLL: {
                struct minmea_sentence_gll frame;
                if (minmea_parse_gll(&frame, line)) {
                    #if GPS_Debug_Print
                      printf(INDENT_SPACES "$xxGLL: lat        : %d\n", (frame.latitude.value));
                      printf(INDENT_SPACES "$xxGLL: long       : %d\n", (frame.longitude.value));
                     printf(INDENT_SPACES "$xxGLL: %d:%d:%d %d\n",
                             frame.time.hours,
                             frame.time.minutes,
                             frame.time.seconds,
                             frame.time.microseconds);
                      printf(INDENT_SPACES "$xxGLL: %F\n",position2float( (frame.latitude.value)));
                      printf(INDENT_SPACES "$xxGLL: %F\n",position2float( (frame.longitude.value)));
                  #endif
                MstrGPSData.lat = position2float(frame.latitude.value);
                MstrGPSData.lng = position2float(frame.longitude.value);
                MstrGPSData.time = frame.time;

                MstrGPSData.GPS_Done = true;
                }
                else {
                    printf(INDENT_SPACES "$xxGLL sentence is not parsed\n");
                }
            } break;

               case MINMEA_SENTENCE_GSA: {
                struct minmea_sentence_gsa frame;
                if (minmea_parse_gsa(&frame, line)) {
                     #if GPS_Debug_Print
                    printf(INDENT_SPACES "$xxGSA: fix type        : %d\n", (frame.fix_type));
                   #endif
              MstrGPSData.fix = frame.fix_type;

                }
                else {
                    printf(INDENT_SPACES "$xxGSA sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GGA: {
                struct minmea_sentence_gga frame;
                if (minmea_parse_gga(&frame, line)) {
                 #if GPS_Debug_Print
                    printf(INDENT_SPACES "$xxGGA: fix quality: %d\n", frame.fix_quality);
                    printf(INDENT_SPACES "$xxGGA: lat        : %F\n",position2float( (frame.latitude.value)));
                    printf(INDENT_SPACES "$xxGGA: long       : %F\n",position2float( (frame.longitude.value)));
                    printf(INDENT_SPACES "$xxGGA: altitude   : %f", minmea_tofloat(&frame.altitude));
                    printf(INDENT_SPACES " %c\n"                  , frame.altitude_units);
                    printf(INDENT_SPACES "$xxGGA: height   : %f  ", minmea_tofloat(&frame.height));
                    printf(INDENT_SPACES " %c\n"                  , frame.height_units);
                    printf(INDENT_SPACES "$xxGGA: %d:%d:%d %d\n",
                           frame.time.hours,
                           frame.time.minutes,
                           frame.time.seconds,
                           frame.time.microseconds);
                 #endif
                 MstrGPSData.alt_m =  minmea_tofloat(&frame.altitude) + minmea_tofloat(&frame.height);
                }
                else {
                    printf(INDENT_SPACES "$xxGGA sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GST: {
                struct minmea_sentence_gst frame;
                if (minmea_parse_gst(&frame, line)) {
                    printf(INDENT_SPACES "$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n",
                            frame.latitude_error_deviation.value, frame.latitude_error_deviation.scale,
                            frame.longitude_error_deviation.value, frame.longitude_error_deviation.scale,
                            frame.altitude_error_deviation.value, frame.altitude_error_deviation.scale);
                    printf(INDENT_SPACES "$xxGST fixed point latitude,longitude and altitude error deviation"
                           " scaled to one decimal place: (%d,%d,%d)\n",
                            minmea_rescale(&frame.latitude_error_deviation, 10),
                            minmea_rescale(&frame.longitude_error_deviation, 10),
                            minmea_rescale(&frame.altitude_error_deviation, 10));
                    printf(INDENT_SPACES "$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
                            minmea_tofloat(&frame.latitude_error_deviation),
                            minmea_tofloat(&frame.longitude_error_deviation),
                            minmea_tofloat(&frame.altitude_error_deviation));
                }
                else {
                    printf(INDENT_SPACES "$xxGST sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GSV: {
                struct minmea_sentence_gsv frame;
                if (minmea_parse_gsv(&frame, line)) {
                  //  printf(INDENT_SPACES "$xxGSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
                    printf(INDENT_SPACES "$xxGSV: sattelites in view: %d\n", frame.total_sats);
                  /*  for (int i = 0; i < 4; i++)
                        printf(INDENT_SPACES "$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                            frame.sats[i].nr,
                            frame.sats[i].elevation,
                            frame.sats[i].azimuth,
                            frame.sats[i].snr);
                            */
                }
                else {
                    printf(INDENT_SPACES "$xxGSV sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_VTG: {
               struct minmea_sentence_vtg frame;
               if (minmea_parse_vtg(&frame, line)) {
                #if GPS_Debug_Print
                    printf(INDENT_SPACES "$xxVTG: true track degrees = %f\n",
                           minmea_tofloat(&frame.true_track_degrees));
                    printf(INDENT_SPACES "        magnetic track degrees = %f\n",
                           minmea_tofloat(&frame.magnetic_track_degrees));
                    printf(INDENT_SPACES "        speed knots = %f\n",
                            minmea_tofloat(&frame.speed_knots));
                    printf(INDENT_SPACES "        speed kph = %f\n",
                            minmea_tofloat(&frame.speed_kph));
               #endif
               MstrGPSData.spd_knts = minmea_tofloat(&frame.speed_knots);
               }
               else {
                    printf(INDENT_SPACES "$xxVTG sentence is not parsed\n");
               }
            } break;

            case MINMEA_SENTENCE_ZDA: {
                struct minmea_sentence_zda frame;
                if (minmea_parse_zda(&frame, line)) {
                    printf(INDENT_SPACES "$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
                           frame.time.hours,
                           frame.time.minutes,
                           frame.time.seconds,
                           frame.date.day,
                           frame.date.month,
                           frame.date.year,
                           frame.hour_offset,
                           frame.minute_offset);
                }
                else {
                    printf(INDENT_SPACES "$xxZDA sentence is not parsed\n");
                }
            } break;

            case MINMEA_INVALID: {
                printf(INDENT_SPACES "$xxxxx sentence is not valid\n");
            } break;

            default: {
                printf(INDENT_SPACES "$xxxxx sentence is not parsed\n");
            } break;
        }
    //}

}

float position2float(int32_t position)
{
  float intpart = 0.0f;
  float decpart = 0.0f;
  float output = 0.0f;
  float inputFloat = ((float) position)/ 10000000.0F;

  intpart = (float)((int)(inputFloat));
  decpart = (inputFloat - intpart)* 100.0F/60.0F;

    output = intpart + decpart;

  return output;

}


void printGPS()
{
  printf("lat          : %f\r\n", MstrGPSData.lat);
  printf("long         : %f\r\n", MstrGPSData.lng);
  printf("alt (m)      : %f\r\n", MstrGPSData.alt_m);
  printf("speed (knts) : %f\r\n", MstrGPSData.spd_knts);
  printf("fix          : %d\r\n", MstrGPSData.fix);
  printf("Time         : %d:%d:%d\n",
       MstrGPSData.time.hours,
       MstrGPSData.time.minutes,
       MstrGPSData.time.seconds);
}
/** @} */
