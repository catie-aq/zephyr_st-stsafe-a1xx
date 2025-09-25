/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>

#include "command_decoder.h"

LOG_MODULE_REGISTER(command_decoder);

void print_perso_info(stse_Handler_t *stse_handler)
{
	uint8_t cmd_count = 0;
	int ret = stsafea_get_command_count(stse_handler, &cmd_count);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get command count: %d", ret);
		return;
	}

	LOG_RAW("=== %d commands configured (not specified commands are FREE) ===\n", cmd_count);

	stse_cmd_authorization_record_t record_table[cmd_count];
	stse_cmd_authorization_CR_t change_rights;
	ret = stsafea_get_command_AC_table(stse_handler, cmd_count, &change_rights, record_table);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get command AC table: %d", ret);
		return;
	}

	for (uint8_t i = 0; i < cmd_count; i++) {
		if ((int)record_table[i].command_AC == (int)STSE_CMD_AC_FREE) {
			continue;
		}

		char *ac_type;
		switch (record_table[i].command_AC) {
		case STSE_CMD_AC_NEVER:
			ac_type = "NEVER";
			break;
		case STSE_CMD_AC_FREE:
			ac_type = "FREE";
			break;
		case STSE_CMD_AC_ADMIN:
			ac_type = "ADMIN";
			break;
		case STSE_CMD_AC_HOST:
			ac_type = "HOST";
			break;
		case STSE_CMD_AC_ADMIN_OR_PWD:
			ac_type = "ADMIN or PASSWORD";
			break;
		case STSE_CMD_AC_ADMIN_OR_HOST:
			ac_type = "ADMIN or HOST";
			break;
		default:
			ac_type = "UNKNOWN";
			break;
		}

		if (record_table[i].extended_header == 0x00) {
			LOG_RAW("\tCmd %02X: AC=%s, CMD_enc=%c, RSP_enc=%c\n",
				record_table[i].header, ac_type,
				record_table[i].host_encryption_flags.cmd ? 'Y' : 'N',
				record_table[i].host_encryption_flags.rsp ? 'Y' : 'N');
		} else {
			LOG_RAW("\tExt Cmd %02X: AC=%s, CMD_enc=%c, RSP_enc=%c\n",
				record_table[i].extended_header, ac_type,
				record_table[i].host_encryption_flags.cmd ? 'Y' : 'N',
				record_table[i].host_encryption_flags.rsp ? 'Y' : 'N');
		}
	}
	LOG_RAW("\n");
}
