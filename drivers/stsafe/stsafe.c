#define DT_DRV_COMPAT st_stsafe_a120

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(stsafe, CONFIG_STSAFE_LOG_LEVEL);

struct stsafe_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec reset_gpio;
};

struct stsafe_data {
	struct k_mutex lock;
};

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

	gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_INACTIVE);
	k_mutex_init(&data->lock);

	LOG_INF("STSAFE skeleton ready at addr 0x%02x on %s", cfg->i2c.addr, cfg->i2c.bus->name);
	return 0;
}

#define STSAFE_INIT(inst)                                                                          \
	static struct stsafe_data stsafe_data_##inst;                                              \
	static const struct stsafe_config stsafe_cfg_##inst = {                                    \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                                 \
		.reset_gpio = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),                            \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, stsafe_init, NULL, &stsafe_data_##inst, &stsafe_cfg_##inst,    \
			      POST_KERNEL, CONFIG_STSAFE_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(STSAFE_INIT)
