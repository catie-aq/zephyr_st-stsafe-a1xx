/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"

stse_ReturnCode_t stse_services_platform_init(void)
{
	stse_platform_crc16_init(NULL);
	gpio_pin_set_dt(&stsafereset, 0);

	k_msleep(1);
	gpio_pin_set_dt(&stsafereset, 1);

	k_msleep(10);

	return STSE_OK;
}

void stse_platform_Delay_ms(PLAT_UI16 delay_val)
{
	k_msleep((PLAT_UI32)delay_val);
}

stse_ReturnCode_t stse_platform_power_on(PLAT_UI8 bus, PLAT_UI8 devAddr)
{
	gpio_pin_set_dt(&stsafereset, 1);
	return (STSE_OK);
}

stse_ReturnCode_t stse_platform_power_off(PLAT_UI8 bus, PLAT_UI8 devAddr)
{
	gpio_pin_set_dt(&stsafereset, 0);
	return (STSE_OK);
}

stse_ReturnCode_t stse_platform_generate_random_init(void *pArg)
{
	(void)pArg; // Unused parameter
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_delay_init(void *pArg)
{
	(void)pArg; // Unused parameter
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_power_init(void *pArg)
{
	(void)pArg; // Unused parameter
	return STSE_OK;
}
