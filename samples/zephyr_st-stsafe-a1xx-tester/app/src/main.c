/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "stselib.h"

#include "helpers/command_decoder.h"

LOG_MODULE_REGISTER(main);

const struct gpio_dt_spec stsafe_reset = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, stsafereset_gpios);

int main(void)
{
	LOG_RAW("************************************************************\n\n");
	LOG_RAW("      Zest Security Secure Element (STSafe-A1xx) Tester     \n\n");
	LOG_RAW("************************************************************\n\n");

	int ret = STSE_OK;

	if (!gpio_is_ready_dt(&stsafe_reset)) {
		LOG_ERR("STSafe reset GPIO is not ready!");
		return -1;
	}

	if (gpio_pin_configure_dt(&stsafe_reset, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_ERR("Failed to configure STSafe reset GPIO!");
		return -1;
	}

#ifdef CONFIG_STSAFE_A110
	LOG_RAW("Using STSafe-A110 chip type.\n\n");
#elif defined(CONFIG_STSAFE_A120)
	LOG_RAW("Using STSafe-A120 chip type.\n\n");
#else
	LOG_ERR("Unsupported STSafe chip type: %d", STSAFE_CHIP_TYPE);
	return -1;
#endif

	stse_Handler_t stse_handler;
	ret = stse_set_default_handler_value(&stse_handler);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to set default handler value: %d", ret);
		return -1;
	}

#ifdef CONFIG_STSAFE_A110
	stse_handler.device_type = STSAFE_A110;
#elif defined(CONFIG_STSAFE_A120)
	stse_handler.device_type = STSAFE_A120;
#endif

	ret = stse_init(&stse_handler);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to initialize STSafe handler: %d", ret);
		return -1;
	}
	LOG_RAW("STSafe handler initialized successfully.\n\n");
	print_perso_info(&stse_handler);

#ifdef CONFIG_STSAFE_A110
	stsafea_host_key_slot_t host_key_slot = {0};
	ret = stsafea_query_host_key(&stse_handler, &host_key_slot);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get host key slot: %d", ret);
		return -1;
	}
	LOG_RAW("Host Key Slot: \n\tPresence Flag: %d \n\tC-MAC Counter: "
		"0x%02X%02X%02X\n\n",
		host_key_slot.key_presence_flag, host_key_slot.cmac_sequence_counter[0],
		host_key_slot.cmac_sequence_counter[1], host_key_slot.cmac_sequence_counter[2]);
#elif defined(CONFIG_STSAFE_A120)
	stsafea_host_key_slot_v2_t host_key_slot_v2 = {0};
	ret = stsafea_query_host_key_v2(&stse_handler, &host_key_slot_v2);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get host key slot v2: %d", ret);
		return -1;
	}
	LOG_RAW("Host Key Slot V2: \n\tPresence Flag: %d\n\tKey Type: %d\n\tC-MAC Counter: "
		"0x%02X%02X%02X%02X\n\n",
		host_key_slot_v2.key_presence_flag, host_key_slot_v2.key_type,
		host_key_slot_v2.cmac_sequence_counter[0],
		host_key_slot_v2.cmac_sequence_counter[1],
		host_key_slot_v2.cmac_sequence_counter[2],
		host_key_slot_v2.cmac_sequence_counter[3]);
#endif

	return 0;
}
