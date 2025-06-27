/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib/core/stse_platform.h"

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

static const struct device *bus_i2c = DEVICE_DT_GET(DT_NODELABEL(sixtron_i2c));

#ifdef CONFIG_STSAFE_A110
LOG_MODULE_REGISTER(STSAFE_A110_I2C);
static uint8_t i2c_buffer[507U];
#elif defined(CONFIG_STSAFE_A120)
LOG_MODULE_REGISTER(STSAFE_A120_I2C);
static uint8_t i2c_buffer[752U];
#else
LOG_MODULE_REGISTER(STSAFE_I2C);
static uint8_t i2c_buffer[507U];
#endif

static uint16_t i2c_frame_size;
static volatile uint16_t i2c_frame_offset;

stse_ReturnCode_t stse_platform_i2c_wake(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed)
{
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_init(PLAT_UI8 busID)
{
	return stse_platform_i2c_wake(busID, 0x20, 100);
}

stse_ReturnCode_t stse_platform_i2c_send_start(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
					       const PLAT_UI16 frameLength)
{
	if (frameLength > sizeof(i2c_buffer) / sizeof(i2c_buffer[0])) {
		LOG_ERR("Frame length exceeds buffer size");
		return STSE_PLATFORM_BUFFER_ERR;
	}
	i2c_frame_size = frameLength;
	i2c_frame_offset = 0;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_continue(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						  PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	if (data_size != 0) {
		if (pData == NULL) {
			memset((i2c_buffer + i2c_frame_offset), 0x00, data_size);
		} else {
			memcpy((i2c_buffer + i2c_frame_offset), pData, data_size);
		}
		i2c_frame_offset += data_size;
	}
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_stop(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
					      PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	stse_ReturnCode_t ret =
		stse_platform_i2c_send_continue(busID, devAddr, speed, pData, data_size);

	if (ret == STSE_OK) {
		ret = i2c_write(bus_i2c, i2c_buffer, i2c_frame_size, devAddr);
	}
	if (ret != STSE_OK) {
		return STSE_PLATFORM_BUS_ACK_ERROR;
	}

	return ret;
}

stse_ReturnCode_t stse_platform_i2c_receive_start(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						  PLAT_UI16 frameLength)
{
	uint8_t stat_len[3];
	uint8_t ret = 0;

	ret = i2c_read(bus_i2c, stat_len, sizeof(stat_len), devAddr);
	if (ret != STSE_OK) {
		LOG_ERR("I2C read failed during receive start");
		return STSE_PLATFORM_BUS_ACK_ERROR;
	}

	i2c_frame_size = ((stat_len[1] << 0) + stat_len[2]) + STSE_RSP_FRAME_HEADER_SIZE +
			 STSE_FRAME_CRC_SIZE;
	frameLength = i2c_frame_size;

	if (i2c_frame_size > sizeof(i2c_buffer) / sizeof(i2c_buffer[0])) {
		return STSE_PLATFORM_BUFFER_ERR;
	}

	ret = i2c_read(bus_i2c, i2c_buffer, frameLength, devAddr);
	if (ret != STSE_OK) {
		return STSE_PLATFORM_BUS_ERR;
	}
	i2c_frame_offset = 0;
	return (stat_len[0] & 0x3F);
}

stse_ReturnCode_t stse_platform_i2c_receive_continue(PLAT_UI8 busID, PLAT_UI8 devAddr,
						     PLAT_UI16 speed, PLAT_UI8 *pData,
						     PLAT_UI16 data_size)
{
	if (pData != NULL) {
		if (i2c_frame_offset == 1) {
			i2c_frame_offset += 2;
		}
		memcpy(pData, (i2c_buffer + i2c_frame_offset), data_size);
	}
	i2c_frame_offset += data_size;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_receive_stop(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						 PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	if (pData != NULL) {
		memcpy(pData, (i2c_buffer + i2c_frame_offset), data_size);
	}

	i2c_frame_offset = 0;
	return STSE_OK;
}
