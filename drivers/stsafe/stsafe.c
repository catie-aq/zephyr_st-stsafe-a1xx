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
};

static int stsafe_reset(const struct device *dev)
{
	const struct stsafe_config *cfg = dev->config;
	int ret = gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		LOG_ERR("Reset GPIO config failed: %d", ret);
		return ret;
	}

	gpio_pin_set_dt(&cfg->reset_gpio, 1);
	k_msleep(1);
	gpio_pin_set_dt(&cfg->reset_gpio, 0);
	k_msleep(10);

	LOG_DBG("STSAFE reset complete");
	return 0;
}

stse_Handle_t *stsafe_acquire(const struct device *dev, k_timeout_t timeout)
{
	struct stsafe_data *data = dev->data;
	if (!data->ready) {
		return NULL;
	}
	if (k_mutex_lock(&data->lock, timeout) != 0) {
		return NULL;
	}
	return &data->handle;
}

void stsafe_release(const struct device *dev)
{
	struct stsafe_data *data = dev->data;
	k_mutex_unlock(&data->lock);
}

static int stsafe_init(const struct device *dev)
{
	const struct stsafe_config *cfg = dev->config;
	struct stsafe_data *data = dev->data;

	if (!device_is_ready(cfg->i2c.bus)) {
		LOG_ERR("I2C bus not ready");
		return -ENODEV;
	}
	if (!gpio_is_ready_dt(&cfg->reset_gpio)) {
		LOG_ERR("Reset GPIO not ready");
		return -ENODEV;
	}

	k_mutex_init(&data->lock);

	stsafe_reset(dev);

	stse_ReturnCode_t rc = stse_set_default_handler_value(&data->handle);
	if (rc != STSE_OK) {
		LOG_ERR("stse_set_default_handler_value failed: 0x%x", rc);
		return -EIO;
	}

	data->handle.io.busID = cfg->bus_id;
	data->handle.device_type = cfg->device_type;

	rc = stse_init(&data->handle, (void *)dev);
	if (rc != STSE_OK) {
		LOG_ERR("stse_init failed: 0x%x", rc);
		return -EIO;
	}

	data->ready = true;
	LOG_INF("STSAFE @0x%02x initialized. Type: %s", cfg->i2c.addr,
		cfg->device_type == STSAFE_A110 ? "A110" : "A120");
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
