/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"
#include <psa/crypto.h>
#include <zephyr/logging/log.h>
#include <zephyr/psa/key_ids.h>

LOG_MODULE_REGISTER(stse_key_store, LOG_LEVEL_INF);

#include "aes.h"

static void secure_zero(void *ptr, size_t len)
{
	volatile uint8_t *p = ptr;
	while (len--) {
		*p++ = 0;
	}
}

static int store_persistent_key(psa_key_id_t id, psa_key_type_t type, size_t bits,
				psa_algorithm_t alg, psa_key_usage_t usage, const uint8_t *key_data,
				size_t key_data_len)
{
	psa_status_t ret;
	psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

	ret = psa_get_key_attributes(id, &attr);
	if (ret == PSA_SUCCESS) {
		LOG_INF("Key ID %u (0x%x) already exists", id, id);
		psa_reset_key_attributes(&attr);
		return 0;
	}

	psa_set_key_lifetime(&attr, PSA_KEY_LIFETIME_PERSISTENT);
	psa_set_key_id(&attr, id);
	psa_set_key_type(&attr, type);
	psa_set_key_bits(&attr, bits);
	psa_set_key_algorithm(&attr, alg);
	psa_set_key_usage_flags(&attr, usage);

	psa_key_id_t imported_id;
	ret = psa_import_key(&attr, key_data, key_data_len, &imported_id);
	if (ret != PSA_SUCCESS) {
		LOG_ERR("Failed to import key %u (0x%x): %d", id, id, ret);
		return -1;
	}

	LOG_INF("Successfully imported key %u (0x%x)", imported_id, imported_id);
	return 0;
}

stse_ReturnCode_t stse_platform_store_session_key(PLAT_UI8 *pCypherKey, PLAT_UI32 *pCypherKeyIdx,
						  PLAT_UI8 *pMACKey, PLAT_UI32 *pMACKeyIdx,
						  PLAT_UI16 key_length)
{

	if (!pCypherKey || !pCypherKeyIdx || !pMACKey || !pMACKeyIdx) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if (!((key_length == 16) || (key_length == 32))) {
		return STSE_CORE_INVALID_PARAMETER;
	}

	int ret = 0;
	ret = store_persistent_key(ITS_ID_KEY_CMAC, PSA_KEY_TYPE_AES, key_length * 8, PSA_ALG_CMAC,
				   PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE,
				   pMACKey, key_length);
	if (ret != 0) {
		return STSE_SESSION_ERROR;
	}

	ret = store_persistent_key(
		ITS_ID_KEY_CBC, PSA_KEY_TYPE_AES, key_length * 8, PSA_ALG_CBC_NO_PADDING,
		PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT, pCypherKey, key_length);
	if (ret != 0) {
		psa_destroy_key(ITS_ID_KEY_CMAC);
		return STSE_SESSION_ERROR;
	}

	store_persistent_key(ITS_ID_KEY_ECB, PSA_KEY_TYPE_AES, key_length * 8,
			     PSA_ALG_ECB_NO_PADDING, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT,
			     pCypherKey, key_length);
	if (ret != 0) {
		psa_destroy_key(ITS_ID_KEY_CMAC);
		psa_destroy_key(ITS_ID_KEY_CBC);
		return STSE_SESSION_ERROR;
	}

	*pCypherKeyIdx = ITS_ID_KEY_CBC;
	*pMACKeyIdx = ITS_ID_KEY_CMAC;

	secure_zero(pCypherKey, key_length);
	secure_zero(pMACKey, key_length);

	return STSE_OK;
}

stse_ReturnCode_t stse_platform_delete_key(PLAT_UI32 CypherKeyIdx, PLAT_UI32 MACKeyIdx)
{
	if (CypherKeyIdx != ITS_ID_KEY_CBC || MACKeyIdx != ITS_ID_KEY_CMAC) {
		return STSE_OK;
	}

	psa_destroy_key(ITS_ID_KEY_CMAC);
	psa_destroy_key(ITS_ID_KEY_CBC);
	psa_destroy_key(ITS_ID_KEY_ECB);

	return STSE_OK;
}
