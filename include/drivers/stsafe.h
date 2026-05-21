/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_STSAFE_H_
#define ZEPHYR_INCLUDE_DRIVERS_STSAFE_H_

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include "stselib.h"

stse_Handle_t *stsafe_get_handle(const struct device *dev);
stse_Handle_t *stsafe_acquire(const struct device *dev, k_timeout_t timeout);
void stsafe_release(const struct device *dev);

#endif /* ZEPHYR_INCLUDE_DRIVERS_STSAFE_H_ */
