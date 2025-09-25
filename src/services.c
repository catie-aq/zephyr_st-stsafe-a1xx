/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"

stse_ReturnCode_t stse_services_platform_init(void)
{
	stse_platform_crc16_init();
	gpio_pin_set_dt(&stsafe_reset, 0);

	k_msleep(1);
	gpio_pin_set_dt(&stsafe_reset, 1);

	k_msleep(50);

	return STSE_OK;
}

void stse_platform_Delay_ms(PLAT_UI32 delay_val)
{
	k_msleep(delay_val);
}

stse_ReturnCode_t stse_platform_power_on(PLAT_UI8 bus, PLAT_UI8 devAddr)
{
	gpio_pin_set_dt(&stsafe_reset, 1);
	return (STSE_OK);
}

stse_ReturnCode_t stse_platform_power_off(PLAT_UI8 bus, PLAT_UI8 devAddr)
{
	gpio_pin_set_dt(&stsafe_reset, 0);
	return (STSE_OK);
}

stse_ReturnCode_t stse_platform_generate_random_init(void)
{
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_delay_init(void)
{
	return STSE_OK;
}
stse_ReturnCode_t stse_platform_power_init(void)
{
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cbc_enc(const PLAT_UI8 *pPlaintext, PLAT_UI16 plaintext_length,
					    PLAT_UI8 *pInitial_value, const PLAT_UI8 *pKey,
					    PLAT_UI16 key_length, PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 *pEncryptedtext_length)
{
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cbc_dec(const PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 encryptedtext_length,
					    PLAT_UI8 *pInitial_value, const PLAT_UI8 *pKey,
					    PLAT_UI16 key_length, PLAT_UI8 *pPlaintext,
					    PLAT_UI16 *pPlaintext_length)
{
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_ecb_enc(const PLAT_UI8 *pPlaintext, PLAT_UI16 plaintext_length,
					    const PLAT_UI8 *pKey, PLAT_UI16 key_length,
					    PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 *pEncryptedtext_length)
{
	return STSE_OK;
}
