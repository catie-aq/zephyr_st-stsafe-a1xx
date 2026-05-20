/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STSE_CONF_H
#define STSE_CONF_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "stse_aes.h"

#define STSE_CONF_STSAFE_A_SUPPORT

#ifdef CONFIG_STSE_FRAME_DEBUG
#define STSE_FRAME_DEBUG_LOG
#endif

#ifdef CONFIG_STSE_USE_HOST_SESSION
#define STSE_CONF_USE_HOST_SESSION
#endif

#ifdef CONFIG_STSE_ECC
#ifdef CONFIG_STSE_ECC_NIST_P_256
#define STSE_CONF_ECC_NIST_P_256
#endif

#ifdef CONFIG_STSE_ECC_NIST_P_384
#define STSE_CONF_ECC_NIST_P_384
#endif

#ifdef CONFIG_STSE_ECC_NIST_P_521
#define STSE_CONF_ECC_NIST_P_521
#endif
#endif

#endif /* STSE_CONF_H */
