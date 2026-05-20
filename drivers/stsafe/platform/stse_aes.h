/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __STSE_AES_H__
#define __STSE_AES_H__

#ifdef CONFIG_STSE_USE_HOST_SESSION
#include <psa/crypto.h>
#include <zephyr/psa/key_ids.h>

#define ITS_BASE_ADDR          ZEPHYR_PSA_APPLICATION_KEY_ID_RANGE_BEGIN
#define STSE_ITS_ID_KEY_CMAC   ITS_BASE_ADDR
#define STSE_ITS_ID_KEY_CIPHER ITS_BASE_ADDR + 1
#define STSE_ITS_ID_KEY_CBC    STSE_ITS_ID_KEY_CIPHER
#define STSE_ITS_ID_KEY_ECB    STSE_ITS_ID_KEY_CIPHER + 1
#endif /* CONFIG_STSE_USE_HOST_SESSION */

#endif /* __STSE_AES_H__ */
