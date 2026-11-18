/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "core/stse_platform.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
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

/* Mirror of stsafe_config from stsafe.c */
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

static struct stsafe_i2c_ctx *get_ctx(PLAT_UI8 busID)
{
	if (busID >= CONFIG_STSAFE_MAX_INSTANCES || !ctx_table[busID].used) {
		LOG_ERR("invalid busID %u", busID);
		return NULL;
	}
	return &ctx_table[busID];
}

stse_ReturnCode_t stse_platform_i2c_wake(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed)
{
	ARG_UNUSED(busID);
	ARG_UNUSED(devAddr);
	ARG_UNUSED(speed);
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

	if (!device_is_ready(cfg->i2c.bus)) {
		LOG_ERR("%s: I2C bus not ready", stsafe_dev->name);
		return STSE_PLATFORM_BUS_ERR;
	}

	ctx_table[busID].i2c_bus = cfg->i2c.bus;
	ctx_table[busID].i2c_addr = cfg->i2c.addr;
	ctx_table[busID].bus_id = cfg->bus_id;
	ctx_table[busID].frame_size = 0;
	ctx_table[busID].frame_offset = 0;
	ctx_table[busID].used = true;

	LOG_DBG("%s: i2c_init bus_id=%u addr=0x%02x", stsafe_dev->name, busID, cfg->i2c.addr);
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_start(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
					       const PLAT_UI16 frameLength)
{
	ARG_UNUSED(devAddr);
	ARG_UNUSED(speed);
	struct stsafe_i2c_ctx *ctx = get_ctx(busID);

	if (ctx == NULL) {
		return STSE_PLATFORM_BUFFER_ERR;
	}
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
	ARG_UNUSED(devAddr);
	ARG_UNUSED(speed);
	struct stsafe_i2c_ctx *ctx = get_ctx(busID);

	if (ctx == NULL) {
		return STSE_PLATFORM_BUFFER_ERR;
	}
	if (data_size == 0) {
		return STSE_OK;
	}
	if ((uint32_t)ctx->frame_offset + data_size > STSAFE_I2C_BUFFER_SIZE) {
		LOG_ERR("send overflow: offset=%u size=%u", ctx->frame_offset, data_size);
		return STSE_PLATFORM_BUFFER_ERR;
	}

	if (pData == NULL) {
		memset(ctx->buffer + ctx->frame_offset, 0x00, data_size);
	} else {
		memcpy(ctx->buffer + ctx->frame_offset, pData, data_size);
	}
	ctx->frame_offset += data_size;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_send_stop(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
					      PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	struct stsafe_i2c_ctx *ctx = get_ctx(busID);
	stse_ReturnCode_t ret;
	int err;

	if (ctx == NULL) {
		return STSE_PLATFORM_BUFFER_ERR;
	}

	ret = stse_platform_i2c_send_continue(busID, devAddr, speed, pData, data_size);
	if (ret != STSE_OK) {
		return ret;
	}

	err = i2c_write(ctx->i2c_bus, ctx->buffer, ctx->frame_size, ctx->i2c_addr);
	if (err < 0) {
		LOG_ERR("i2c_write failed on bus_id=%u addr=0x%02x: %d", busID, ctx->i2c_addr, err);
		return STSE_PLATFORM_BUS_ACK_ERROR;
	}

	LOG_DBG("frame sent on bus_id=%u addr=0x%02x, length=%u", busID, ctx->i2c_addr,
		ctx->frame_size);
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_receive_start(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						  PLAT_UI16 frameLength)
{
	ARG_UNUSED(devAddr);
	ARG_UNUSED(speed);
	struct stsafe_i2c_ctx *ctx = get_ctx(busID);
	int err;

	if (ctx == NULL) {
		return STSE_PLATFORM_BUFFER_ERR;
	}
	if (frameLength > STSAFE_I2C_BUFFER_SIZE) {
		LOG_ERR("frame length %u exceeds buffer size %u", frameLength,
			STSAFE_I2C_BUFFER_SIZE);
		return STSE_PLATFORM_BUFFER_ERR;
	}

	ctx->frame_size = frameLength;
	err = i2c_read(ctx->i2c_bus, ctx->buffer, frameLength, ctx->i2c_addr);
	if (err < 0) {
		/* Expected while the STSAFE is busy: STSELib polls until ACK */
		LOG_DBG("i2c_read NACK on bus_id=%u addr=0x%02x: %d", busID, ctx->i2c_addr, err);
		return STSE_PLATFORM_BUS_ACK_ERROR;
	}

	ctx->frame_offset = 0;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_receive_continue(PLAT_UI8 busID, PLAT_UI8 devAddr,
						     PLAT_UI16 speed, PLAT_UI8 *pData,
						     PLAT_UI16 data_size)
{
	ARG_UNUSED(devAddr);
	ARG_UNUSED(speed);
	struct stsafe_i2c_ctx *ctx = get_ctx(busID);

	if (ctx == NULL) {
		return STSE_PLATFORM_BUFFER_ERR;
	}

	if (pData != NULL) {
		if ((uint32_t)ctx->frame_offset + data_size > ctx->frame_size) {
			LOG_ERR("receive overflow: offset=%u size=%u frame=%u", ctx->frame_offset,
				data_size, ctx->frame_size);
			return STSE_PLATFORM_BUFFER_ERR;
		}
		memcpy(pData, ctx->buffer + ctx->frame_offset, data_size);
	}
	ctx->frame_offset += data_size;
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_i2c_receive_stop(PLAT_UI8 busID, PLAT_UI8 devAddr, PLAT_UI16 speed,
						 PLAT_UI8 *pData, PLAT_UI16 data_size)
{
	ARG_UNUSED(devAddr);
	ARG_UNUSED(speed);
	struct stsafe_i2c_ctx *ctx = get_ctx(busID);

	if (ctx == NULL) {
		return STSE_PLATFORM_BUFFER_ERR;
	}

	if (pData != NULL) {
		if ((uint32_t)ctx->frame_offset + data_size > ctx->frame_size) {
			LOG_ERR("receive overflow: offset=%u size=%u frame=%u", ctx->frame_offset,
				data_size, ctx->frame_size);
			ctx->frame_offset = 0;
			return STSE_PLATFORM_BUFFER_ERR;
		}
		memcpy(pData, ctx->buffer + ctx->frame_offset, data_size);
	}
	ctx->frame_offset = 0;

	LOG_DBG("frame received on bus_id=%u addr=0x%02x, length=%u", busID, ctx->i2c_addr,
		ctx->frame_size);
	return STSE_OK;
}
