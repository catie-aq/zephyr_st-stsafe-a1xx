/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"
#include <psa/crypto.h>

#define STSE_INVALID_KEY_HANDLE 0

static void secure_zero(void *ptr, size_t len)
{
	volatile uint8_t *p = ptr;
	while (len--) {
		*p++ = 0;
	}
}

stse_ReturnCode_t stse_platform_store_session_key(PLAT_UI8 *pCypherKey, PLAT_UI8 *pCypherKeyIdx,
						  PLAT_UI8 *pMACKey, PLAT_UI8 *pMACKeyIdx,
						  PLAT_UI16 key_length)
{
	if (!pCypherKey || !pCypherKeyIdx || !pMACKey || !pMACKeyIdx) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if (!((key_length == 16) || (key_length == 32))) {
		return STSE_CORE_INVALID_PARAMETER;
	}

	psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
	psa_status_t st;

	psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
	psa_set_key_bits(&attr, key_length * 8);
	psa_set_key_algorithm(&attr,
			      PSA_ALG_CMAC | PSA_ALG_ECB_NO_PADDING | PSA_ALG_CBC_NO_PADDING);
	psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT |
					       PSA_KEY_USAGE_SIGN_MESSAGE |
					       PSA_KEY_USAGE_VERIFY_MESSAGE);

	psa_key_id_t cipher_id = PSA_KEY_ID_NULL;
	psa_key_id_t mac_id = PSA_KEY_ID_NULL;

	st = psa_import_key(&attr, pCypherKey, key_length, &cipher_id);
	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

	st = psa_import_key(&attr, pMACKey, key_length, &mac_id);
	if (st != PSA_SUCCESS) {
		psa_destroy_key(cipher_id);
		return STSE_SESSION_ERROR;
	}

	secure_zero(pCypherKey, key_length);
	secure_zero(pMACKey, key_length);

	*pCypherKeyIdx = (PLAT_UI8)cipher_id;
	*pMACKeyIdx = (PLAT_UI8)mac_id;

	return STSE_OK;
}

stse_ReturnCode_t stse_platform_delete_key(PLAT_UI8 CypherKeyIdx, PLAT_UI8 MACKeyIdx)
{
	if (CypherKeyIdx != STSE_INVALID_KEY_HANDLE) {
		psa_destroy_key((psa_key_id_t)CypherKeyIdx);
	}

	if (MACKeyIdx != STSE_INVALID_KEY_HANDLE) {
		psa_destroy_key((psa_key_id_t)MACKeyIdx);
	}

	return STSE_OK;
}
