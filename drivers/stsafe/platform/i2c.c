/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "core/stse_platform.h"
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include "drivers/stsafe.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(stsafe, CONFIG_STSAFE_LOG_LEVEL);

/*
 * The STSELib platform layer needs a per-instance scratch buffer to assemble
 * frames before sending them on the wire (and to receive responses). The buffer
 * size depends on the chip variant:
 *
 *   - STSAFE-A110: up to 507 bytes per frame
 *   - STSAFE-A120: up to 752 bytes per frame
 *
 * The driver supports both variants in the same firmware, so we size every
 * buffer to the maximum. This costs 245 bytes per A110 instance.
 */
#define STSAFE_I2C_BUFFER_SIZE 752U

// Mirror of stsafe_config from stsafe.c
struct stsafe_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec reset_gpio;
	int bus_id;
	uint8_t device_type;
};

struct stsafe_i2c_ctx {
	const struct device *i2c_bus;
	uint16_t i2c_addr;
	uint8_t buffer[STSAFE_I2C_BUFFER_SIZE];
	uint16_t frame_size;
	uint16_t frame_offset;
	int bus_id;
	bool used;
};

static struct stsafe_i2c_ctx ctx_table[CONFIG_STSAFE_MAX_INSTANCES];

stse_ReturnCode_t stse_platform_i2c_wake(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed)
{
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_init(PLAT_UI8 busID, void *pArg)
{
	if (busID >= CONFIG_STSAFE_MAX_INSTANCES) {
		LOG_ERR("busID %u out of range (max %u)", busID, CONFIG_STSAFE_MAX_INSTANCES);
		return STSE_PLATFORM_BUFFER_ERR;
	}
	if (pArg == NULL) {
		LOG_ERR("NULL device pointer");
		return STSE_PLATFORM_BUFFER_ERR;
	}

	const struct device *stsafe_dev = (const struct device *)pArg;
	const struct stsafe_config *cfg = stsafe_dev->config;

	ctx_table[busID].i2c_bus = cfg->i2c.bus;
	ctx_table[busID].i2c_addr = cfg->i2c.addr;
	ctx_table[busID].used = true;
	ctx_table[busID].bus_id = cfg->bus_id;

	LOG_DBG("%s: i2c_init bus_id=%u addr=0x%02x", stsafe_dev->name, busID, cfg->i2c.addr);
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_start(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
					       const PLAT_UI16 frameLength)
{
	if (busID >= CONFIG_STSAFE_MAX_INSTANCES || !ctx_table[busID].used) {
		LOG_ERR("invalid busID %u", busID);
		return STSE_PLATFORM_BUFFER_ERR;
	}
	struct stsafe_i2c_ctx *ctx = &ctx_table[busID];

	if (frameLength > STSAFE_I2C_BUFFER_SIZE) {
		LOG_ERR("frame length %u exceeds buffer size %u", frameLength,
			STSAFE_I2C_BUFFER_SIZE);
		return STSE_PLATFORM_BUFFER_ERR;
	}
	ctx->frame_size = frameLength;
	ctx->frame_offset = 0;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_continue(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						  PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	struct stsafe_i2c_ctx *ctx = &ctx_table[busID];

	if (data_size != 0) {
		if (pData == NULL) {
			memset(ctx->buffer + ctx->frame_offset, 0x00, data_size);
		} else {
			memcpy(ctx->buffer + ctx->frame_offset, pData, data_size);
		}
		ctx->frame_offset += data_size;
	}
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_stop(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
					      PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	struct stsafe_i2c_ctx *ctx = &ctx_table[busID];

	stse_ReturnCode_t ret =
		stse_platform_i2c_send_continue(busID, ctx->i2c_addr, speed, pData, data_size);
	if (ret == STSE_OK) {
		ret = i2c_write(ctx->i2c_bus, ctx->buffer, ctx->frame_size, ctx->i2c_addr);
	}
	if (ret != STSE_OK) {
		LOG_ERR("failed to send frame on bus_id=%u addr=0x%02x: %d", busID, ctx->i2c_addr,
			ret);
		return STSE_PLATFORM_BUS_ACK_ERROR;
	}

	LOG_DBG("frame sent successfully on bus_id=%u addr=0x%02x, length=%u", busID, ctx->i2c_addr,
		ctx->frame_size);
	return ret;
}

stse_ReturnCode_t stse_platform_i2c_receive_start(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						  PLAT_UI16 frameLength)
{
	struct stsafe_i2c_ctx *ctx = &ctx_table[busID];
	uint8_t stat_len[3];
	uint8_t ret;

	ret = i2c_read(ctx->i2c_bus, stat_len, sizeof(stat_len), ctx->i2c_addr);
	if (ret != STSE_OK) {
		LOG_ERR("i2c_read (header) failed on bus_id=%u addr=0x%02x: %d", busID,
			ctx->i2c_addr, ret);
		return STSE_PLATFORM_BUS_ACK_ERROR;
	}
	ctx->frame_size = ((stat_len[1] << 8) + stat_len[2]) + STSE_RSP_FRAME_HEADER_SIZE +
			  STSE_FRAME_CRC_SIZE;
	frameLength = ctx->frame_size;

	if (ctx->frame_size > STSAFE_I2C_BUFFER_SIZE) {
		LOG_ERR("frame size %u exceeds buffer size %u", ctx->frame_size,
			STSAFE_I2C_BUFFER_SIZE);
		return STSE_PLATFORM_BUFFER_ERR;
	}

	ret = i2c_read(ctx->i2c_bus, ctx->buffer, frameLength, ctx->i2c_addr);
	if (ret != STSE_OK) {
		LOG_ERR("i2c_read (data) failed on bus_id=%u addr=0x%02x: %d", busID, ctx->i2c_addr,
			ret);
		return STSE_PLATFORM_BUS_ERR;
	}
	ctx->frame_offset = 0;
	return (stat_len[0] & 0x3F);
}

stse_ReturnCode_t stse_platform_i2c_receive_continue(PLAT_UI8 busID, PLAT_UI8 devAddr,
						     PLAT_UI16 speed, PLAT_UI8 *pData,
						     PLAT_UI16 data_size)
{
	struct stsafe_i2c_ctx *ctx = &ctx_table[busID];

	if (pData != NULL) {
		if (ctx->frame_offset == 1) {
			ctx->frame_offset += 2;
		}
		memcpy(pData, ctx->buffer + ctx->frame_offset, data_size);
	}
	ctx->frame_offset += data_size;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_receive_stop(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						 PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	struct stsafe_i2c_ctx *ctx = &ctx_table[busID];

	if (pData != NULL) {
		memcpy(pData, ctx->buffer + ctx->frame_offset, data_size);
	}
	ctx->frame_offset = 0;

	LOG_DBG("frame received successfully on bus_id=%u addr=0x%02x, length=%u", busID,
		ctx->i2c_addr, ctx->frame_size);
	return STSE_OK;
}
