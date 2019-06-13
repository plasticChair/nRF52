#include "paper.h"

#include <string.h>

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"

#define PAPER_RST_PIN 19
#define PAPER_DC_PIN  18
#define PAPER_BUSY_PIN 17

#define PAPER_SPI_INSTANCE 0
#define PAPER_SPI_SS_PIN   20
#define PAPER_SPI_MOSI_PIN 23
#define PAPER_SPI_SCK_PIN  25

#define PAPER_BYTE_WIDTH ((PAPER_PIXEL_WIDTH % 8 == 0) ? PAPER_PIXEL_WIDTH / 8 : PAPER_PIXEL_WIDTH / 8 + 1)
#define PAPER_BYTE_HEIGHT PAPER_PIXEL_HEIGHT
#define PAPER_BUFLEN PAPER_BYTE_WIDTH * PAPER_BYTE_HEIGHT



static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(PAPER_SPI_INSTANCE);
//static const nrf_drv_spi_t spi = NRFX_SPI_INSTANCE(PAPER_SPI_INSTANCE);

static uint8_t paper_POWER_SETTING[] = {
  POWER_SETTING,
  0x07, 0x00, 0x0a, 0x00, // WAS : 0x07, 0x00, 0x08, 0x00
};

static uint8_t paper_BOOSTER_SOFT_START[] = {
  BOOSTER_SOFT_START,
  0x07, 0x07, 0x07,
};

static uint8_t paper_POWER_ON[] = {
  POWER_ON,
 // 0x00,0x00,0x00
};

static uint8_t paper_PANEL_SETTING[] = {
  PANEL_SETTING,
  0xCF, // 0x0d,  //WAS: 0xbf, 0x0d
};
// red
static uint8_t paper_VCOM_AND_DATA_INTERVAL_SETTING[] = {
  VCOM_AND_DATA_INTERVAL_SETTING,
  0x17, // 0x0d,  //WAS: 0xbf, 0x0d
};

static uint8_t paper_PLL_CONTROL[] = {
  PLL_CONTROL,
  0x39 // WAS: 0x39 when temp > 30
};

static uint8_t paper_TCON_RESOLUTION[] = {
  TCON_RESOLUTION,
  0xC8,
  0x00,
  0xC8,
};

static uint8_t paper_VCM_DC_SETTING_REGISTER[] = {
  VCM_DC_SETTING_REGISTER,
  0x0E,
};


static uint8_t paper_DISPLAY_REFRESH[] = {
  DISPLAY_REFRESH
};

uint8_t lut_vcom0[] =
{ 0x20, 
    0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
    0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00
};

uint8_t lut_w[] =
{ 0x21,
    0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04
};

uint8_t lut_b[] = 
{
    0x22,
    0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04
};

uint8_t lut_g1[] = 
{
0x23,
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};

uint8_t lut_g2[] = 
{
0x24,
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};

uint8_t lut_vcom1[] = 
{
0x25,
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t lut_red0[] = 
{
0x26,
    0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t lut_red1[] = 
{
0x27,
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static uint8_t paper_buffer_BW[PAPER_BUFLEN];
static uint8_t paper_buffer_R[PAPER_BUFLEN];

static void paper_begin_command(void) {
  nrf_gpio_pin_clear(PAPER_DC_PIN);
}

static void paper_begin_data(void) {
  nrf_gpio_pin_set(PAPER_DC_PIN);
}

static ret_code_t paper_tx_cmd(uint8_t *cmd, uint8_t length) {
  ret_code_t ret;

  paper_begin_command();
  ret = nrf_drv_spi_transfer(&spi, cmd, 1, NULL, 0);
  if (ret != NRF_SUCCESS) return ret;

  if (length - 1 > 0) {
    paper_begin_data();
    ret = nrf_drv_spi_transfer(&spi, cmd + 1, length - 1, NULL, 0);
    if (ret != NRF_SUCCESS) return ret;
  }

  return ret;
}


static void paper_reset(void) {
  nrf_gpio_pin_clear(PAPER_RST_PIN);
  nrf_delay_ms(200);
  nrf_gpio_pin_set(PAPER_RST_PIN);
  nrf_delay_ms(200);
}

static ret_code_t paper_tx_lut(void) {
  ret_code_t ret;

  ret = paper_tx_cmd(lut_vcom0, sizeof(lut_vcom0));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(lut_w, sizeof(lut_w));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(lut_b, sizeof(lut_b));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(lut_g1, sizeof(lut_g1));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(lut_g2, sizeof(lut_g2));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(lut_vcom1, sizeof(lut_vcom1));
  if (ret != NRF_SUCCESS) return ret;

    ret = paper_tx_cmd(lut_red0, sizeof(lut_red0));
  if (ret != NRF_SUCCESS) return ret;

    ret = paper_tx_cmd(lut_red1, sizeof(lut_red1));
  if (ret != NRF_SUCCESS) return ret;

  return ret;
}

static ret_code_t _paper_display(void) {
  ret_code_t ret;

  /*
   * TODO transfer chunks of 256 bytes rather than calling nrf_drv_spi_transfer
   * for each byte.
   */
   uint8_t temp = 0;
   uint8_t temp2 = 0;
uint8_t temp3 = 0;
  uint8_t command[1];

  uint8_t tempData = 0;
  uint8_t tempData2 = 0;

  command[0] = 0x10;
  ret = paper_tx_cmd(command, 1);
  if (ret != NRF_SUCCESS) return ret;
   nrf_delay_ms(2);

  paper_begin_data();
  for (int i = 0; i < PAPER_BUFLEN; i++) {

//    for (int bit = 0; bit < 4; bit++) {
//      if (((paper_buffer_BW[i]) & (0x80 >> bit)) != 0) {
//        temp |= 0xC0 >> (bit * 2);
//        }
//      }
//
//    ret = nrf_drv_spi_transfer(&spi, &temp , 1, NULL, 0);
//    if (ret != NRF_SUCCESS) return ret;
//
//    temp = 0x00;
//    for (int bit = 4; bit < 8; bit++) {
//      if (((paper_buffer_R[i]) & (0x80 >> bit)) != 0) {
//        temp |= 0xC0 >> ((bit - 4) * 2);
//      }
//    }
//    ret = nrf_drv_spi_transfer(&spi, &temp , 1, NULL, 0);
//    if (ret != NRF_SUCCESS) return ret;

//    tempData = (*(paper_buffer_BW + i) & 0xF0) >> 4 ;
//    ret = nrf_drv_spi_transfer(&spi, &tempData , 1, NULL, 0);
//    if (ret != NRF_SUCCESS) return ret;
//
//    tempData2 = (*(paper_buffer_BW + i) & 0xF) ;
//    ret = nrf_drv_spi_transfer(&spi, &tempData, 1, NULL, 0);
//    if (ret != NRF_SUCCESS) return ret;



    tempData = (*(paper_buffer_BW + i) & 0xF0) >> 4 ;
    temp = 0x00;
    temp2 = 0x00;
    temp3 = 0x00;
    for (int bit = 0; bit < 4; bit++) {
      temp2 =  (tempData & (1 << bit));
      temp3 =  (temp2 << 1);
      if (bit != 0)
        temp |=  (temp2|temp3) << (bit)  ;
        else
          temp =  (temp2|temp3) ;
    }
    ret = nrf_drv_spi_transfer(&spi, &temp , 1, NULL, 0);
    if (ret != NRF_SUCCESS) return ret;

    tempData2 = (*(paper_buffer_BW + i) & 0xF) ;
    temp = 0x00;
    temp2 = 0x00;
    temp3 = 0x00;
      for (int bit = 0; bit < 4; bit++) {
      temp2 =  (tempData2 & (1 << bit));
      temp3 =  (temp2 << 1);
      if (bit != 0)
        temp |=  (temp2|temp3) << (bit)  ;
        else
          temp =  (temp2|temp3)   ;
    }
    ret = nrf_drv_spi_transfer(&spi, &temp, 1, NULL, 0);
    if (ret != NRF_SUCCESS) return ret;


  }

  nrf_delay_ms(2);

  /*
   * TODO i'm not yet sure about the purpose of sending the previous buffer too.
   * just sending 0x00s works pretty good. when sending the actual previous
   * image buffer, it looks like sections that have not changed become kinda
   * dirty / greyish (which could be useful because one can use three colors).
   */

  // memcpy(paper_prev_buffer, paper_buffer, PAPER_BUFLEN);

  command[0] = 0x13;
  ret = paper_tx_cmd(command, 1);
  if (ret != NRF_SUCCESS) return ret;

  paper_begin_data();
  for (int i = 0; i < PAPER_BUFLEN; i++) {
    ret = nrf_drv_spi_transfer(&spi, paper_buffer_R + i, 1, NULL, 0);
    if (ret != NRF_SUCCESS) return ret;
  }


  //nrf_delay_ms(10);

 command[0] = 0x11;
  ret = paper_tx_cmd(command, 1);
  if (ret != NRF_SUCCESS) return ret;

  //ret = paper_tx_lut();
  //if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_DISPLAY_REFRESH, sizeof(paper_DISPLAY_REFRESH));
  if (ret != NRF_SUCCESS) return ret;

  nrf_delay_ms(300); // TODO use busy pin rather than arbitrary delays
  //paper_waitBusy();



  return ret;
}


/* ------------- INIT ------------------- */
ret_code_t paper_init(void) {
  ret_code_t ret;

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin   = PAPER_SPI_SS_PIN;
  spi_config.mosi_pin = PAPER_SPI_MOSI_PIN;
  spi_config.sck_pin  = PAPER_SPI_SCK_PIN;

  ret = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
  //ret = nrfx_spim_init(&spi, &spi_config, NULL, NULL);
  if (ret != NRF_SUCCESS) return ret;

  nrf_gpio_cfg_output(PAPER_RST_PIN);
  nrf_gpio_cfg_output(PAPER_DC_PIN);
  nrf_gpio_cfg_input(PAPER_BUSY_PIN, GPIO_PIN_CNF_PULL_Disabled);

  paper_reset();

  ret = paper_tx_cmd(paper_POWER_SETTING, sizeof(paper_POWER_SETTING));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_BOOSTER_SOFT_START, sizeof(paper_BOOSTER_SOFT_START));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_POWER_ON, sizeof(paper_POWER_ON));
  if (ret != NRF_SUCCESS) return ret;


  nrf_delay_ms(100);
  //paper_waitBusy();

  ret = paper_tx_cmd(paper_PANEL_SETTING, sizeof(paper_PANEL_SETTING));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_VCOM_AND_DATA_INTERVAL_SETTING, sizeof(paper_VCOM_AND_DATA_INTERVAL_SETTING));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_PLL_CONTROL, sizeof(paper_PLL_CONTROL));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_TCON_RESOLUTION, sizeof(paper_TCON_RESOLUTION));
  if (ret != NRF_SUCCESS) return ret;

  ret = paper_tx_cmd(paper_VCM_DC_SETTING_REGISTER, sizeof(paper_VCM_DC_SETTING_REGISTER));
  if (ret != NRF_SUCCESS) return ret;

  paper_tx_lut();

  memset(paper_buffer_BW, 0xFF, PAPER_BUFLEN);
  memset(paper_buffer_R, 0xFF, PAPER_BUFLEN);

  return ret;
}

void paper_uninit(void) {
  // TODO uninit spi and free buffers
}

void paper_pixel_draw(uint16_t x, uint16_t y, uint32_t color) {
  ASSERT(x < PAPER_PIXEL_WIDTH);
  ASSERT(y < PAPER_PIXEL_HEIGHT);

//x = PAPER_PIXEL_WIDTH - x - 1;
//y = PAPER_PIXEL_HEIGHT - y -1;

// uint16_t i = x / 8 + y * PAPER_PIXEL_WIDTH / 8;
//
// paper_buffer_BW[i] = (paper_buffer_BW[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
//  paper_buffer_R[i] = (paper_buffer_R[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
//  if (color == 0) return;
//  else if (color == 1) paper_buffer_BW[i] = (paper_buffer_BW[i] | (1 << (7 - x % 8)));
//  else if (color == 2) paper_buffer_R[i] = (paper_buffer_R[i] | (1 << (7 - x % 8)));




//  int bitn = y * PAPER_PIXEL_WIDTH + x;
 // int byten = bitn / 8;
//  bitn = 8 - bitn % 8 - 1;



    uint16_t byten = x / 8 + y * PAPER_PIXEL_WIDTH / 8;
    uint16_t bitn = 8 - (x % 8) -  1;


    paper_buffer_BW[byten] |= (1UL << (bitn));
    paper_buffer_R[byten]  |= (1UL << (bitn));

  switch (color){


    case 1: //black
      paper_buffer_BW[byten] &= ~(1UL << bitn);
      
      break;

    case 2: //red
      paper_buffer_R[byten] &= ~(1UL << (bitn));
      break;

    default:
      break;
  }


  //if (color == 0) paper_buffer[byten] |= 1UL << (bitn);
  //else 
}

void paper_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  ASSERT(x + width <= PAPER_PIXEL_WIDTH);
  ASSERT(y + height <= PAPER_PIXEL_HEIGHT);
 
 for (uint16_t _x = x; _x < x + width; _x++)
    for (uint16_t _y = y; _y < y + height; _y++)
      paper_pixel_draw(_x, _y, color);
}

void paper_display(void) {
  APP_ERROR_CHECK(_paper_display());
}

void paper_rotation_set(nrf_lcd_rotation_t rotation) {
  UNUSED_PARAMETER(rotation);

  // TODO
}

void paper_display_invert(bool invert) {
  UNUSED_PARAMETER(invert);

  // TODO
}



void  paper_drawBitmapBM(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
 
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

 for (int16_t j = 0; j < h; j++)
    {
      for (int16_t i = 0; i < w; i++ )
      {


        if (i & 7) byte <<= 1;
        else
        {
          byte = bitmap[j * byteWidth + i / 8];
        }
        // keep using overwrite mode
        
          if ((byte & 0x80))
        //if (!(byte & 0x80))
        {
          uint16_t xd = x + i;
          uint16_t yd = y + j;
   
          paper_pixel_draw(xd, yd, color);
        }
      }
    }
}


void paper_print(nrf_lcd_t const * p_instance, uint8_t x, uint8_t y, char *text, uint8_t color, uint8_t size){

      nrf_gfx_point_t text_start = NRF_GFX_POINT(x,y);
      if (size == 0)
          //APP_ERROR_CHECK(nrf_gfx_print(p_instance, &text_start, color, text, p_font, true));
          APP_ERROR_CHECK(nrf_gfx_print(p_instance, &text_start, color, text, p_font_6b, true));
      else if (size == 1)
      APP_ERROR_CHECK(nrf_gfx_print(p_instance, &text_start, color, text, p_font_8, true));
      else if (size == 2)
      APP_ERROR_CHECK(nrf_gfx_print(p_instance, &text_start, color, text, p_font_10b, true));
        
}


void paper_waitBusy()
{
 //0: busy, 1: idle
  while(!nrf_gpio_pin_read(PAPER_BUSY_PIN));
    nrf_delay_ms(1);
}