/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"

#ifdef CONFIG_STSE_USE_HOST_SESSION
#include <psa/crypto.h>
#endif

stse_ReturnCode_t stse_platform_crypto_init(void)
{
#ifdef CONFIG_STSE_USE_HOST_SESSION
	static bool inited = false;
	if (!inited) {
		psa_status_t st = psa_crypto_init();
		if (st != PSA_SUCCESS) {
			return st;
		}
		inited = true;
	}
	return PSA_SUCCESS;
#else
	return STSE_OK;
#endif
}
