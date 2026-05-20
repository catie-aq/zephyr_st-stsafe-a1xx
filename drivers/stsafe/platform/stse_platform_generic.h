/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OWN_STSE_PLATFORM_GENERIC_H
#define OWN_STSE_PLATFORM_GENERIC_H

#include <zephyr/kernel.h>

#define PLAT_UI8           uint8_t
#define PLAT_UI16          uint16_t
#define PLAT_UI32          uint32_t
#define PLAT_UI64          uint64_t
#define PLAT_I8            int8_t
#define PLAT_I16           int16_t
#define PLAT_I32           int32_t
#define PLAT_PACKED_STRUCT __PACKED

#ifdef CONFIG_STSE_USE_RSP_POLLING
#define STSE_USE_RSP_POLLING
#define STSE_MAX_POLLING_RETRY      CONFIG_STSE_MAX_POLLING_RETRY
#define STSE_FIRST_POLLING_INTERVAL CONFIG_STSE_FIRST_POLLING_INTERVAL
#define STSE_POLLING_RETRY_INTERVAL CONFIG_STSE_POLLING_RETRY_INTERVAL
#endif

#endif /* OWN_STSE_PLATFORM_GENERIC_H */
