/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stsafe_a120

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "stselib.h"

LOG_MODULE_REGISTER(stsafe, CONFIG_STSAFE_LOG_LEVEL);

struct stsafe_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec reset_gpio;
	int bus_id;
	uint8_t device_type;
};

struct stsafe_data {
	stse_Handle_t handle;
	struct k_mutex lock;
	bool ready;

	enum stsafe_mode {
		STSAFE_MODE_UNSET = 0,
		STSAFE_MODE_SIMPLE,
		STSAFE_MODE_LOCKED,
	} mode;
	struct k_spinlock mode_lock;
};

static int stsafe_reset(const struct device *dev)
{
	const struct stsafe_config *cfg = dev->config;
	int ret = gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		LOG_ERR("%s: reset GPIO config failed: %d", dev->name, ret);
		return ret;
	}

	gpio_pin_set_dt(&cfg->reset_gpio, 1);
	k_msleep(1);
	gpio_pin_set_dt(&cfg->reset_gpio, 0);
	k_msleep(10);

	LOG_DBG("%s: reset complete", dev->name);
	return 0;
}

static bool stsafe_claim_mode(struct stsafe_data *data, enum stsafe_mode target)
{
	bool ok = false;

	K_SPINLOCK(&data->mode_lock) {
		if (data->mode == STSAFE_MODE_UNSET || data->mode == target) {
			data->mode = target;
			ok = true;
		}
	}
	return ok;
}

stse_Handle_t *stsafe_get_handle(const struct device *dev)
{
	struct stsafe_data *data = dev->data;

	if (!data->ready) {
		LOG_ERR("%s: get_handle called on uninitialized device", dev->name);
		return NULL;
	}
	if (!stsafe_claim_mode(data, STSAFE_MODE_SIMPLE)) {
		LOG_ERR("%s: get_handle called on device already in locked mode "
			"(use acquire/release instead)",
			dev->name);
		return NULL;
	}
	return &data->handle;
}

stse_Handle_t *stsafe_acquire(const struct device *dev, k_timeout_t timeout)
{
	struct stsafe_data *data = dev->data;
	if (!data->ready) {
		LOG_ERR("%s: acquire called on uninitialized device", dev->name);
		return NULL;
	}

	if (!stsafe_claim_mode(data, STSAFE_MODE_LOCKED)) {
		LOG_ERR("%s: acquire called on device already in simple mode "
			"(use get_handle instead)",
			dev->name);
		return NULL;
	}

	if (k_mutex_lock(&data->lock, timeout) != 0) {
		LOG_ERR("%s: acquire timed out", dev->name);
		return NULL;
	}
	LOG_DBG("%s: acquired", dev->name);
	return &data->handle;
}

void stsafe_release(const struct device *dev)
{
	struct stsafe_data *data = dev->data;
	k_mutex_unlock(&data->lock);
	LOG_DBG("%s: released", dev->name);
}

static int stsafe_init(const struct device *dev)
{
	const struct stsafe_config *cfg = dev->config;
	struct stsafe_data *data = dev->data;

	if (!device_is_ready(cfg->i2c.bus)) {
		LOG_ERR("%s: I2C bus '%s' not ready", dev->name, cfg->i2c.bus->name);
		return -ENODEV;
	}
	if (!gpio_is_ready_dt(&cfg->reset_gpio)) {
		LOG_ERR("%s: reset GPIO port '%s' not ready", dev->name,
			cfg->reset_gpio.port->name);
		return -ENODEV;
	}

	k_mutex_init(&data->lock);

	stsafe_reset(dev);

	stse_ReturnCode_t rc = stse_set_default_handler_value(&data->handle);
	if (rc != STSE_OK) {
		LOG_ERR("%s: stse_set_default_handler_value failed: 0x%x", dev->name, rc);
		return -EIO;
	}

	data->handle.io.busID = cfg->bus_id;
	data->handle.device_type = cfg->device_type;

	rc = stse_init(&data->handle, (void *)dev);
	if (rc != STSE_OK) {
		LOG_ERR("%s: stse_init failed: 0x%x", dev->name, rc);
		return -EIO;
	}

	data->ready = true;
	LOG_INF("%s: ready (A1%s @ 0x%02x, bus_id=%d)", dev->name,
		cfg->device_type == STSAFE_A110 ? "10" : "20", cfg->i2c.addr, cfg->bus_id);
	return 0;
}

#define GET_STSAFE_TYPE(inst)                                                                      \
	COND_CODE_1(DT_INST_NODE_HAS_COMPAT(inst, st_stsafe_a110), (STSAFE_A110), (STSAFE_A120))

#define STSAFE_INIT(inst)                                                                          \
	static struct stsafe_data stsafe_data_##inst;                                              \
	static const struct stsafe_config stsafe_cfg_##inst = {                                    \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                                 \
		.reset_gpio = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),                            \
		.bus_id = inst,                                                                    \
		.device_type = GET_STSAFE_TYPE(inst),                                              \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, stsafe_init, NULL, &stsafe_data_##inst, &stsafe_cfg_##inst,    \
			      POST_KERNEL, CONFIG_STSAFE_INIT_PRIORITY, NULL);

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT st_stsafe_a120
DT_INST_FOREACH_STATUS_OKAY(STSAFE_INIT)
#undef DT_DRV_COMPAT

#define DT_DRV_COMPAT st_stsafe_a110
DT_INST_FOREACH_STATUS_OKAY(STSAFE_INIT)
#undef DT_DRV_COMPAT
