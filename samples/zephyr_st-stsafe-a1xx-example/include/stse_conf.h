/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STSE_CONF_H
#define STSE_CONF_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define STSE_CONF_STSAFE_A_SUPPORT

// #define STSE_FRAME_DEBUG_LOG

/**
 * Secure channel configuration
 */
#define STSE_CONF_USE_HOST_SESSION

/**
 * RSP Polling retry setup
 */
#define STSE_USE_RSP_POLLING
#define STSE_MAX_POLLING_RETRY      10
#define STSE_FIRST_POLLING_INTERVAL 33
#define STSE_POLLING_RETRY_INTERVAL 100

/**
 * STSafe-A1xx reset GPIO pin configuration
 */
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
extern const struct gpio_dt_spec stsafe_reset;

#endif /* STSE_CONF_H */
