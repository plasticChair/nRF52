/*
Copyright 2017 Knut Auvor Grythe

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "bme280_twi.h"
#include "sdk_errors.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define _REG_CTRL_HUM  0xF2U
#define _REG_CTRL_MEAS 0xF4U
#define _REG_CONFIG    0xF5U
#define _REG_TEMP_MSB  0xFAU
#define _REG_TEMP_LSB  0xFBU
#define _REG_TEMP_XLSB 0xFCU

#define _REG_PRES_MSB  0xF7U
#define _REG_HUMD_MSB  0xFDU

#define _REG_DIG_T1_LSB 0x88U
#define _REG_DIG_T1_MSB 0x89U
#define _REG_DIG_T2_LSB 0x8AU
#define _REG_DIG_T2_MSB 0x8BU
#define _REG_DIG_T3_LSB 0x8CU
#define _REG_DIG_T3_MSB 0x8DU
#define _REG_DIG_P1_LSB 0x8E

#define _REG_DIG_H1_LSB 0x8E
#define _REG_DIG_H2_LSB 0xE1


#define _SHIFT_OSRS_T 0x05U
#define _SHIFT_OSRS_P 0x02U
#define _SHIFT_T_SB   0x05U
#define _SHIFT_FILTER 0x02U

static volatile bool    m_xfer_done = false;
static volatile uint8_t m_rx_reg = 0;
static int32_t  m_temp_raw;
static int32_t  m_pres_raw;
static int32_t  m_humid_raw;

static const nrf_drv_twi_t *m_twi_ptr;
static bme280_twi_evt_handler_t m_handler;
static uint8_t	m_addr;
static void *   m_context;
static uint8_t  m_buf[20];

static bme280_cal_data_t BMEcalData;
static uint8_t  m_ctrl_meas;

static volatile bool m_measurement_fetched;

int32_t cal_t_fine = 0;

__STATIC_INLINE int32_t _merge_20_bit(uint8_t *buf) {
	return (((uint32_t)buf[0]) << 12 | ((uint32_t)buf[1]) << 4 | buf[2] >> 4);
}

__STATIC_INLINE int16_t _merge_16_bit_MSB(uint8_t *buf) {
	return (((uint16_t)buf[0]) << 8 | buf[1]);
}

__STATIC_INLINE int16_t _merge_16_bit_LSB(uint8_t *buf) {
	return (((uint16_t)buf[1]) << 8 | buf[0]);
}

__STATIC_INLINE void _send_event(bme280_twi_evt_type_t event_type)
{
	bme280_twi_evt_t event = {
		.type = event_type
	};
	m_handler(&event, m_context);
}


static void _compensate_temp(int32_t adc_T, float *temp)
{
	int32_t var1 = ((((adc_T>>3) - ((int32_t)BMEcalData.dig_T1<<1))) * ((int32_t)BMEcalData.dig_T2)) >> 11;
	int32_t var2 = (((((adc_T>>4) - ((int32_t)BMEcalData.dig_T1)) * ((adc_T>>4) - ((int32_t)BMEcalData.dig_T1))) >> 12) *
			((int32_t)BMEcalData.dig_T3)) >> 14;
        cal_t_fine =  (int32_t)(((var1 + var2) * 5 + 128) >> 8);

        *temp = (float)cal_t_fine / 100.0;
}

static void _compensate_humidity(int32_t adc_T, float *humid)
{
	int32_t var1;
	var1 = (cal_t_fine - ((int32_t)76800));
	var1 = (((((adc_T << 14) - (((int32_t)BMEcalData.dig_H4) << 20) - (((int32_t)BMEcalData.dig_H5) * var1)) +
	((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)BMEcalData.dig_H6)) >> 10) * (((var1 * ((int32_t)BMEcalData.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)BMEcalData.dig_H2) + 8192) >> 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)BMEcalData.dig_H1)) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);

	*humid = (float)(var1>>12) / 1024.0;
        
}

static void _compensate_press(int32_t adc_T, float *pres)
{

    double var1;
    double var2;
    double var3;
    double pressure;
    double pressure_min = 30000.0;
    double pressure_max = 110000.0;

    var1 = ((double)cal_t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)BMEcalData.dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)BMEcalData.dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)BMEcalData.dig_P4) * 65536.0);
    var3 = ((double)BMEcalData.dig_P3) * var1 * var1 / 524288.0;
    var1 = (var3 + ((double)BMEcalData.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)BMEcalData.dig_P1);

    /* avoid exception caused by division by zero */
    
    if (var1)
    {
        pressure = 1048576.0 - (double) adc_T;
        pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
        var1 = ((double)BMEcalData.dig_P9) * pressure * pressure / 2147483648.0;
        var2 = pressure * ((double)BMEcalData.dig_P8) / 32768.0;
        pressure = pressure + (var1 + var2 + ((double)BMEcalData.dig_P7)) / 16.0;
        if (pressure < pressure_min)
        {
            pressure = pressure_min;
        }
        else if (pressure > pressure_max)
        {
            pressure = pressure_max;
        }
    }
    else /* Invalid case */
    
    {
        pressure = pressure_min;
    }

	*pres = (float)pressure/100 + 300;
}

static void _write(uint8_t reg, uint8_t data) {
	m_xfer_done = false;
	uint8_t buf[2] = {reg, data};
	ret_code_t err_code = nrf_drv_twi_tx(m_twi_ptr, m_addr, buf, sizeof(buf), false);
	APP_ERROR_CHECK(err_code);
}

static void _write_blocking(uint8_t reg, uint8_t data) {
	_write(reg, data);
	do {
		__WFE();
	} while (m_xfer_done == false);
}

static void _read(uint8_t reg, uint8_t *buf, int len) {
	m_xfer_done = false;
	m_rx_reg = reg;
	nrf_drv_twi_xfer_desc_t desc = {
		.type = NRF_DRV_TWI_XFER_TXRX,
		.address = m_addr,
		.primary_length = sizeof(reg),
		.secondary_length = sizeof(*buf) * len,
		.p_primary_buf = &reg,
		.p_secondary_buf = buf
	};
	ret_code_t err_code = nrf_drv_twi_xfer(m_twi_ptr, &desc, 0);
	APP_ERROR_CHECK(err_code);
}

static void _read_blocking(uint8_t reg, uint8_t *buf, int len) {
	_read(reg, buf, len);
	do {
		__WFE();
	} while (m_xfer_done == false);
}

void bme280_twi_evt_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type) {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TXRX) {
                switch (m_rx_reg) {
                    case _REG_TEMP_MSB:
                        m_temp_raw = _merge_20_bit(m_buf);
                        break;
                    case _REG_PRES_MSB:
                        m_pres_raw = _merge_20_bit(m_buf);
                        break;
                    case _REG_HUMD_MSB:
                        m_humid_raw = _merge_16_bit_MSB(m_buf);
                        break;
                 }
                m_rx_reg = 0;
            }
            m_xfer_done = true;
            break;
        default:
          break;
    }
}

void bme280_twi_init(nrf_drv_twi_t const *       p_twi,
                     bme280_twi_config_t const * p_config)
{
	m_twi_ptr = p_twi;

	m_addr = p_config->addr;

        // Cal Temp data
	_read_blocking(_REG_DIG_T1_LSB, m_buf, 6);
	BMEcalData.dig_T1 = _merge_16_bit_LSB(m_buf);
	BMEcalData.dig_T2 = _merge_16_bit_LSB(m_buf + 2);
	BMEcalData.dig_T3 = _merge_16_bit_LSB(m_buf + 4);
        
        // Cal press data
        _read_blocking(_REG_DIG_P1_LSB, m_buf, 18);
	BMEcalData.dig_P1 = _merge_16_bit_LSB(m_buf);
	BMEcalData.dig_P2 = _merge_16_bit_LSB(m_buf + 2);
	BMEcalData.dig_P3 = _merge_16_bit_LSB(m_buf + 4);
        BMEcalData.dig_P4 = _merge_16_bit_LSB(m_buf + 6);
        BMEcalData.dig_P5 = _merge_16_bit_LSB(m_buf + 8);
        BMEcalData.dig_P6 = _merge_16_bit_LSB(m_buf + 10);
        BMEcalData.dig_P7 = _merge_16_bit_LSB(m_buf + 12);
        BMEcalData.dig_P8 = _merge_16_bit_LSB(m_buf + 14);
        BMEcalData.dig_P9 = _merge_16_bit_LSB(m_buf + 16);

        // Cal humidity data
        _read_blocking(_REG_DIG_H1_LSB, m_buf, 1);
	BMEcalData.dig_H1 =m_buf[0];
        _read_blocking(_REG_DIG_H2_LSB, m_buf, 7);
	BMEcalData.dig_H2 = _merge_16_bit_LSB(m_buf);
	BMEcalData.dig_H3 = m_buf[2];
	BMEcalData.dig_H4 = (m_buf[3] << 4) | (m_buf[4] & 0x0F);
        BMEcalData.dig_H5 = ((m_buf[4] & 0xF0) << 4) | (m_buf[5]);
        BMEcalData.dig_H6 = m_buf[6];

        NRF_LOG_INFO("%d ", BMEcalData.dig_H1);
        NRF_LOG_INFO("%d ", BMEcalData.dig_H2);
        NRF_LOG_INFO("%d ", BMEcalData.dig_H3);
        NRF_LOG_INFO("%d ", BMEcalData.dig_H4);
        NRF_LOG_INFO("%d ", BMEcalData.dig_H5);
        NRF_LOG_INFO("%d ", BMEcalData.dig_H6);



	// Write CONFIG first, because it is only guaranteed to take effect in sleep mode.
	_write_blocking(_REG_CONFIG, ((p_config->standby << _SHIFT_T_SB) | (p_config->filter << _SHIFT_FILTER)));

	// Write CTRL_HUM next, because it only takes effect after writing CTRL_MEAS.
	_write_blocking(_REG_CTRL_HUM, p_config->humidOverSample);

	// Calculate, but don't write CTRL_MEAS yet. It will be written by bme280_twi_enable.
	m_ctrl_meas = ((p_config->temp_oversampling << _SHIFT_OSRS_T)
			| (p_config->pressOverSample << _SHIFT_OSRS_P)
			| BME280_TWI_MODE_NORMAL);
}

void bme280_twi_enable(void)
{
	// Write CTRL_MEAS to start the show
    _write_blocking(_REG_CTRL_MEAS, m_ctrl_meas);
}

void bme280_twi_measurement_fetch(void)
{
    _read_blocking(_REG_TEMP_MSB, m_buf, 3);
}

void bme280_twi_measurement_get(bme280_twi_data_t *data)
{
    _compensate_temp(m_temp_raw, &data->temp);
}

void bme280_getData(bme280_twi_data_t *data)
{

    _read_blocking(_REG_TEMP_MSB, m_buf, 3);
    _compensate_temp(m_temp_raw, &data->temp);

    _read_blocking(_REG_PRES_MSB, m_buf, 3);
    _compensate_press(m_temp_raw, &data->pres);

    _read_blocking(_REG_HUMD_MSB, m_buf, 2);
    _compensate_humidity(m_humid_raw, &data->humidity);


}