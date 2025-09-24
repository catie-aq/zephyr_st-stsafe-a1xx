/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STSE_CONF_H
#define STSE_CONF_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define STSE_CONF_STSAFE_A_SUPPORT

#ifdef CONFIG_STSE_FRAME_DEBUG
#define STSE_FRAME_DEBUG_LOG
#endif

#ifdef CONFIG_STSE_USE_HOST_SESSION
#define STSE_CONF_USE_HOST_SESSION
#endif

#define STSE_USE_RSP_POLLING
#define STSE_MAX_POLLING_RETRY      10
#define STSE_FIRST_POLLING_INTERVAL 33
#define STSE_POLLING_RETRY_INTERVAL 100

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
extern const struct gpio_dt_spec stsafe_reset;

#endif /* STSE_CONF_H */
