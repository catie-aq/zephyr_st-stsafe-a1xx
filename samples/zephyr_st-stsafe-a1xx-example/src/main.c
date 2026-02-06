/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define SHOULD_SEED_HOST_KEY_SLOT 0
#define SHOULD_SEED_HKP_CTRL      0

#include "stselib.h"
const struct gpio_dt_spec stsafe_reset = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, stsafereset_gpios);

LOG_MODULE_REGISTER(main);

#ifdef CONFIG_STSAFE_A110
static stsafea_aes_128_host_keys_t host_key_128 = {
	.host_mac_key = {0x9c, 0x7f, 0xb2, 0xba, 0x15, 0x92, 0x3c, 0xab, 0x8a, 0x01, 0x00, 0x87,
			 0xb6, 0xe4, 0xae, 0x05},
	.host_cipher_key = {0x22, 0x3d, 0xf1, 0x50, 0x12, 0xf1, 0xf2, 0xff, 0x03, 0xd6, 0x01, 0xd9,
			    0x92, 0xd1, 0x31, 0x93},
};

#elif defined(CONFIG_STSAFE_A120)
static stsafea_aes_256_host_keys_t host_key_256 = {
	.host_mac_key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
			 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
			 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f},
	.host_cipher_key = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,
			    0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
			    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f},
};
#endif

static const char *ac_str(uint8_t v)
{
	switch (v) {
	case STSE_AC_ALWAYS:
		return "ALW";
	case STSE_AC_HOST:
		return "HOST";
	case STSE_AC_AUTH:
		return "AUTH";
	case STSE_AC_NEVER:
		return "NEV";
	default:
		return "UNK";
	}
}

static const char *zone_str(uint8_t v)
{
	return (v == 0) ? "SIMPLE" : "COUNTER";
}

int main(void)
{
	LOG_RAW("************************************************************\n\n");
	LOG_RAW("      Zest Security Secure Element (STSafe-A1xx) Sample     \n\n");
	LOG_RAW("************************************************************\n\n");

	if (!gpio_is_ready_dt(&stsafe_reset)) {
		LOG_ERR("STSafe reset GPIO is not ready!");
		return -1;
	}

	if (gpio_pin_configure_dt(&stsafe_reset, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_ERR("Failed to configure STSafe reset GPIO!");
		return -1;
	}

#ifdef CONFIG_STSAFE_A110
	LOG_INF("STSafe-A110 Secure Element Sample Application");
#elif defined(CONFIG_STSAFE_A120)
	LOG_INF("STSafe-A120 Secure Element Sample Application");
#else
	LOG_ERR("Unsupported STSafe Secure Element variant");
	return -1;
#endif

	int ret = STSE_OK;
	stse_Handler_t stse_handler;
	stse_session_t stse_session;

	ret = stse_set_default_handler_value(&stse_handler);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to set default handler value: %d", ret);
		return ret;
	}

#ifdef CONFIG_STSAFE_A110
	stse_handler.device_type = STSAFE_A110;
#elif defined(CONFIG_STSAFE_A120)
	stse_handler.device_type = STSAFE_A120;
#endif

	ret = stse_init(&stse_handler);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to initialize STSafe handler: %d", ret);
		return ret;
	}
	LOG_INF("STSafe handler initialized successfully\n");

#ifdef CONFIG_STSAFE_A110
	stsafea_host_key_slot_t host_key_slot = {0};
	ret = stsafea_query_host_key(&stse_handler, &host_key_slot);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get host key slot: %d", ret);
		return -1;
	}

	if (host_key_slot.key_presence_flag == 0 && SHOULD_SEED_HOST_KEY_SLOT) {
		LOG_INF("Host key slot is empty, seeding with default key");

		ret = stse_host_key_provisioning(&stse_handler, STSAFEA_AES_128_HOST_KEY,
						 (stsafea_host_keys_t *)&host_key_128);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to provision host key: %d", ret);
			return ret;
		}

		ret = stsafea_query_host_key(&stse_handler, &host_key_slot);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to query host key slot: %d", ret);
			return ret;
		}
		LOG_INF("Host key slot seeding completed");
	}

	if (host_key_slot.key_presence_flag == 1) {
		/**
		 * Host key slot already contains a key.
		 * Open the host session.
		 */
		LOG_INF("Host key slot already contains a key");
		stse_session.context.host.key_type = STSE_AES_128_KT;
		ret = stsafea_open_host_session(&stse_handler, &stse_session,
						host_key_128.host_mac_key,
						host_key_128.host_cipher_key);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to open host session: %d", ret);
			return ret;
		}
		LOG_INF("Host session opened successfully\n");
	} else {
		LOG_INF("Host key slot is empty and SHOULD_SEED_HOST_KEY_SLOT is false");
		return 0;
	}
#elif defined(CONFIG_STSAFE_A120)
	/**
	 * Query the host key slot and provision it if empty.
	 */
	stsafea_host_key_slot_v2_t host_key_slot = {0};
	ret = stsafea_query_host_key_v2(&stse_handler, &host_key_slot);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to query host key slot: %d", ret);
		return ret;
	}

	stsafea_host_key_provisioning_ctrl_fields_t stse_hkp_ctrl = {0};
	ret = stsafea_query_host_key_provisioning_ctrl_fields(&stse_handler, &stse_hkp_ctrl);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to query host key provisioning control fields: %d", ret);
		return ret;
	}
	LOG_INF("Current Host key provisioning control fields");
	LOG_RAW("\t Change Right: %d\n", stse_hkp_ctrl.change_right);
	LOG_RAW("\t Filler: %d\n", stse_hkp_ctrl.filler);
	LOG_RAW("\t Plaintext: %d\n", stse_hkp_ctrl.plaintext);
	LOG_RAW("\t Reprovision: %d\n", stse_hkp_ctrl.reprovision);
	LOG_RAW("\t Wrapped Anonymous: %d\n", stse_hkp_ctrl.wrapped_anonymous);
	LOG_RAW("\t Wrapped or DH Derived Authentication Key: %d\n",
		stse_hkp_ctrl.wrapped_or_DH_derived_authentication_key);

	LOG_INF("SHOULD_SEED_HOST_KEY_SLOT: %d", SHOULD_SEED_HOST_KEY_SLOT);
	LOG_INF("SHOULD_SEED_HKP_CTRL: %d", SHOULD_SEED_HKP_CTRL);

	if (SHOULD_SEED_HKP_CTRL) {
		LOG_INF("Seeding host key provisioning control fields");

		stse_hkp_ctrl.change_right = 0;                                // default: 1
		stse_hkp_ctrl.plaintext = 0;                                   // default: 1
		stse_hkp_ctrl.reprovision = 1;                                 // default: 0
		stse_hkp_ctrl.wrapped_anonymous = 1;                           // default: 0
		stse_hkp_ctrl.filler = 0;                                      // default: 0
		stse_hkp_ctrl.wrapped_or_DH_derived_authentication_key = 0xFF; // default: 0xff

		ret = stsafea_put_host_key_provisioning_ctrl_fields(&stse_handler, &stse_hkp_ctrl);

		if (ret != STSE_OK) {
			if (ret == STSE_ACCESS_CONDITION_NOT_SATISFIED) {
				LOG_WRN("Provisioning ctrl fields, Access condition not satisfied");
			} else {
				LOG_WRN("Provisioning ctrl fields, Unknown error %d", ret);
			}
			return 0;
		} else {
			LOG_INF("Host key provisioning control fields updated successfully");
		}
	}

	if (host_key_slot.key_presence_flag == 0 && SHOULD_SEED_HOST_KEY_SLOT) {
		LOG_INF("Host key slot is empty, seeding with default key");

		ret = stse_host_key_provisioning(&stse_handler, STSAFEA_AES_256_HOST_KEY,
						 (stsafea_host_keys_t *)&host_key_256);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to provision host key: %d", ret);
			return ret;
		}

		ret = stsafea_query_host_key_v2(&stse_handler, &host_key_slot);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to query host key slot: %d", ret);
			return ret;
		}
		LOG_INF("Host key slot seeding completed");

		ret = stsafea_generate_wrap_unwrap_key(&stse_handler, 0, STSE_AES_256_KT);
		if (ret == STSE_ACCESS_CONDITION_NOT_SATISFIED) {
			LOG_WRN("Access condition not satisfied for wrap/unwrap key generation");
		}

		if (ret != STSE_OK) {
			LOG_ERR("Failed to generate wrap/unwrap key: %d", ret);
			return ret;
		}

		LOG_INF("Wrap/unwrap key generated successfully\n");
	}

	if (host_key_slot.key_presence_flag == 1) {
		/**
		 * Host key slot already contains a key.
		 * Open the host session.
		 */
		LOG_INF("Host key slot already contains a key");
		stse_session.context.host.key_type = STSE_AES_256_KT;
		ret = stsafea_open_host_session(&stse_handler, &stse_session,
						host_key_256.host_mac_key,
						host_key_256.host_cipher_key);
		if (ret != STSE_OK) {
			LOG_ERR("Failed to open host session: %d", ret);
			return ret;
		}
		LOG_INF("Host session opened successfully\n");
	} else {
		LOG_INF("Host key slot is empty and SHOULD_SEED_HOST_KEY_SLOT is false");
		return 0;
	}
#endif

	LOG_INF("Wrapping data");
	uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t wrapped_data[16] = {0};

	ret = stsafea_wrap_payload(&stse_handler, 0, data, sizeof(data), wrapped_data,
				   sizeof(wrapped_data));
	if (ret != STSE_OK) {
		LOG_ERR("Failed to wrap payload: %d", ret);
		return ret;
	}
	LOG_INF("Data wrapped successfully");
	LOG_HEXDUMP_INF(wrapped_data, sizeof(wrapped_data), "Wrapped Data");

	uint8_t device_id[11] = {0};
	ret = stse_get_device_id(&stse_handler, 0, device_id);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get device ID: %d", ret);
		return ret;
	}
	LOG_HEXDUMP_INF(device_id, sizeof(device_id), "Device ID for cert zone 0");

	uint16_t size = 0;
	ret = stse_get_device_certificate_size(&stse_handler, 0, &size);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get certificate zone size: %d", ret);
		return ret;
	}
	LOG_INF("Certificate zone size: %d bytes", size);

	// uint8_t out[1000] = {0};
	// ret = stse_get_device_certificate(&stse_handler, 0, size, out);
	// // if (ret != STSE_OK) {
	// // 	LOG_ERR("Failed to get device certificate: %d", ret);
	// // 	return ret;
	// // }
	// LOG_HEXDUMP_INF(out, size, "Device Certificate for cert zone 0");

	uint8_t tt_partition_count = 0;
	ret = stsafea_get_total_partition_count(&stse_handler, &tt_partition_count);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get total partition count: %d", ret);
		return ret;
	}

	stsafea_data_partition_record_t partition_records[tt_partition_count];
	uint16_t record_table_length = sizeof(partition_records);

	ret = stsafea_get_data_partitions_configuration(&stse_handler, tt_partition_count,
							partition_records, record_table_length);
	if (ret != STSE_OK) {
		LOG_ERR("Failed to get data partitions configuration: %d", ret);
		return ret;
	}

	LOG_INF("%d partitions", tt_partition_count);
	LOG_RAW("IDX | TYPE    | LEN  | R-AC  | U-AC  | CNT\n");
	LOG_RAW("--- | ------- | ---- | ----- | ----- | ----\n");

	for (uint8_t i = 0; i < tt_partition_count; i++) {
		LOG_RAW("%3d | %-7s | %-4d | %-5s | %-5s | %d\n", partition_records[i].index,
			zone_str(partition_records[i].zone_type),
			partition_records[i].data_segment_length,
			ac_str(partition_records[i].read_ac_cr),
			ac_str(partition_records[i].update_ac_cr),
			partition_records[i].counter_value);
	}

	LOG_RAW("─────────────────────────────────────────────\n");

	PLAT_UI32 test_zone_index = 1;
	PLAT_UI16 test_offset = 0;
	PLAT_UI16 test_length = 32;

	uint8_t write_buffer[32] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44,
				    0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC,
				    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
				    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7};
	uint8_t read_buffer[32] = {0};

	stsafea_update_option_t update_opt = {0};
	stsafea_read_option_t read_opt = {0};

	ret = stsafea_update_data_zone(&stse_handler, test_zone_index, update_opt, test_offset,
				       write_buffer, test_length, STSE_NO_PROT);

	if (ret != STSE_OK) {
		LOG_ERR("Update failed zone %d: %d", test_zone_index, ret);
		return ret;
	} else {
		LOG_INF("Update success zone %d", test_zone_index);
	}

	ret = stsafea_read_data_zone(&stse_handler, test_zone_index, read_opt, test_offset,
				     read_buffer, test_length, STSE_NO_PROT);

	if (ret != STSE_OK) {
		LOG_ERR("Read failed zone %d: %d", test_zone_index, ret);
	} else {
		LOG_INF("Read success zone %d", test_zone_index);
		LOG_HEXDUMP_INF(read_buffer, test_length, "Read Data");

		if (memcmp(write_buffer, read_buffer, test_length) == 0) {
			LOG_INF("Data Match");
		} else {
			LOG_ERR("Data Mismatch");
		}
	}

	return ret;
}
