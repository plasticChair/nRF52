/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
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
 * @defgroup tw_sensor_example main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */

#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "driver/bme280_twi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

bme280_twi_data_t data;
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(0);


void twi_init(void)
{
	const nrf_drv_twi_config_t twi_config = {
		.scl                = 27,
		.sda                = 26,
		.frequency          = NRF_TWI_FREQ_100K,
		.interrupt_priority = APP_IRQ_PRIORITY_HIGH,
		.clear_bus_init     = false
	};

	ret_code_t err_code = nrf_drv_twi_init(&m_twi, &twi_config, bme280_twi_evt_handler, NULL);
	APP_ERROR_CHECK(err_code);

	nrf_drv_twi_enable(&m_twi);
}

void bme280_init(void) {
	const bme280_twi_config_t bme280_twi_config = {
		.addr = BME280_TWI_ADDR_1,
		.standby = BME280_TWI_STANDBY_1000_MS,
		.filter = BME280_TWI_FILTER_2,
		.temp_oversampling = BME280_TWI_OVERSAMPLING_X4,
                .pressOverSample = BME280_TWI_OVERSAMPLING_X4,
                .humidOverSample = BME280_TWI_OVERSAMPLING_X4,
	};

	bme280_twi_init(&m_twi, &bme280_twi_config);
	bme280_twi_enable();
}

static void log_temp(void)
{
	//bme280_twi_data_t data;
	//bme280_twi_measurement_get(&data);

	NRF_LOG_INFO("Temperature: " NRF_LOG_FLOAT_MARKER " degreels Celsius.",
	NRF_LOG_FLOAT((data.temp)));
        NRF_LOG_INFO("Press: " NRF_LOG_FLOAT_MARKER " hPa.",
        NRF_LOG_FLOAT((data.pres)))
        NRF_LOG_INFO("Humidity: " NRF_LOG_FLOAT_MARKER " %.",
        NRF_LOG_FLOAT((data.humidity)))
	NRF_LOG_FLUSH();
}

int main(void)
{
	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
        NRF_LOG_DEFAULT_BACKENDS_INIT();
        
	NRF_LOG_INFO("BME280 TWI sensor example\r\n");
	NRF_LOG_FLUSH();

	twi_init();
	bme280_init();

	while (true) {
            nrf_delay_ms(2000);

            //bme280_twi_measurement_fetch();
            bme280_getData(&data);
            log_temp();
	}
   }
/** @} */
