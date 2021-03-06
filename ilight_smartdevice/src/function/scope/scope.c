/* ------------------------------------------
 * Copyright (c) 2017, Synopsys, Inc. All rights reserved.

 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:

 * 1) Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.

 * 3) Neither the name of the Synopsys, Inc., nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
--------------------------------------------- */

/**
 * \file
 * \ingroup EMBARC_APP_FREERTOS_IOT_ILIGHT_SMARTDEVICE
 * \brief virtual scope for iLight
 */

/**
 * \addtogroup  EMBARC_APP_FREERTOS_IOT_ILIGHT_SMARTDEVICE
 * @{
 */

/*embARC HAL*/
#include "embARC.h"
#include "embARC_debug.h"

/*custom HAL*/
#include "scope.h"

/* scope operation */
static DEV_UART *scope_uart;

/**
 * \brief	CRC check
 * \details	Software crc check.
 * \param	pbuf: the buffer of date to crc check,crc_cnt: the number of bytes to crc check
 * \param	the result of crc check
 */
static uint16_t crc_check(uint8_t *pbuf, uint8_t crc_cnt)
{
	uint16_t crc_temp = 0xFFFF;
	uint8_t i, j;

	for (i = 0; i < crc_cnt; i++ ) {
		crc_temp ^= pbuf[i];

		for (j = 0; j < 8; j++ ) {
			if (crc_temp & 0x01){
				crc_temp = (crc_temp >> 1) ^ 0xA001;
			} else{
				crc_temp = crc_temp >> 1;
			}
		}
	}
	return (crc_temp);
}

/**
 * \brief	Initial the scope
 * \details	Initial the scope using uart0
 * \param[in]	baudrate: the baudrate of uart0
 * \param[out]	-1 for error, 0 for succeed
 */
uint32_t scope_init(uint32_t baudrate)
{
	scope_uart = uart_get_dev(DW_UART_0_ID);

	if (scope_uart == NULL) {
		EMBARC_PRINTF("Failed to get device of uart0 for scope.\r\n");
		return -1;
	}

	if (scope_uart->uart_open(baudrate) == E_OPNED) {
		scope_uart->uart_control(UART_CMD_SET_BAUD, (void *)(baudrate));
		EMBARC_PRINTF("Succeed to initial the uart0 for scope.\r\n");
	}

	return 0;
}

/**
 * \brief	Output the date to scope
 * \details	Output the date to scope.
 * \param[in]	out_data: four date sended to scope
 * \param[out]	-1 for error, 0 for succeed
 */
uint32_t output_data(int16_t *out_data)
{
	uint8_t data_buf[10];
	uint16_t temp[4] = {0};
	uint8_t i;
	uint16_t crc16 = 0;

	for (i = 0; i < 4; i++ ) {
		temp[i] = (uint16_t)out_data[i];
	}

	for (i = 0; i < 4; i++ ) {
		data_buf[i * 2] = (uint8_t)(temp[i] % 256);
		data_buf[i * 2 + 1] = (uint8_t)(temp[i] / 256);
	}

	crc16 = crc_check(data_buf, 8);
	data_buf[8] = crc16 % 256;
	data_buf[9] = crc16 / 256;

	return scope_uart->uart_write(data_buf, 10);
}

/** @} */