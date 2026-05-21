/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

#include <drivers/stsafe.h>

#include "helpers/command_decoder.h"

int main(void)
{
	LOG_RAW("************************************************************\n\n");
	LOG_RAW("      Zest Security Secure Element (STSafe-A1xx) Tester     \n\n");
	LOG_RAW("************************************************************\n\n");

	int ret = STSE_OK;

	const struct device *se = DEVICE_DT_GET(DT_NODELABEL(stsafe_1_20));
	if (!device_is_ready(se)) {
		LOG_ERR("STSAFE device not ready");
		return -ENODEV;
	}

	LOG_INF("STSAFE device is ready: %s", se->name);
	stse_Handle_t *stse_handle = stsafe_get_handle(se);
	if (!stse_handle) {
		LOG_ERR("Could not acquire STSAFE handle");
		return -EBUSY;
	}

	uint8_t echo[8] = {'t', 'e', 's', 't', 't', 'e', 's', 't'};
	uint8_t echo_reply[sizeof(echo)] = {0};
	ret = stse_device_echo(stse_handle, echo, echo_reply, sizeof(echo_reply));

	if (ret != STSE_OK) {
		LOG_ERR("stse_device_echo failed: 0x%x. Function is probably locked", ret);
	}

	LOG_HEXDUMP_INF(echo_reply, sizeof(echo_reply), "Echo reply from STSAFE:");

	print_perso_info(stse_handle);

	if (stse_handle->device_type == STSAFE_A100) {
		stsafea_host_key_slot_t host_key_slot = {0};
		ret = stsafea_query_host_key(stse_handle, &host_key_slot);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to get host key slot: %d", ret);
			return -1;
		}
		LOG_RAW("Host Key Slot: \n\tPresence Flag: %d \n\tC-MAC Counter: "
			"0x%02X%02X%02X\n\n",
			host_key_slot.key_presence_flag, host_key_slot.cmac_sequence_counter[0],
			host_key_slot.cmac_sequence_counter[1],
			host_key_slot.cmac_sequence_counter[2]);
	} else if (stse_handle->device_type == STSAFE_A120) {
		stsafea_host_key_slot_v2_t host_key_slot_v2 = {0};
		ret = stsafea_query_host_key_v2(stse_handle, &host_key_slot_v2);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to get host key slot v2: %d", ret);
			return -1;
		}
		LOG_RAW("Host Key Slot V2: \n\tPresence Flag: %d\n\tKey Type: %d\n\tC-MAC "
			"Counter: "
			"0x%02X%02X%02X%02X\n\n",
			host_key_slot_v2.key_presence_flag, host_key_slot_v2.key_type,
			host_key_slot_v2.cmac_sequence_counter[0],
			host_key_slot_v2.cmac_sequence_counter[1],
			host_key_slot_v2.cmac_sequence_counter[2],
			host_key_slot_v2.cmac_sequence_counter[3]);
	}

	return 0;
}
