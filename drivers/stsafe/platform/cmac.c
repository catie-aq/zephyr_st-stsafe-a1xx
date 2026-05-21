/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(stsafe, CONFIG_STSAFE_LOG_LEVEL);

#include "stselib.h"

#include <psa/crypto.h>
typedef struct {
	psa_key_id_t key_id;
	psa_mac_operation_t op;
} stsafea1xx_psa_cmac_ctx_t;
static stsafea1xx_psa_cmac_ctx_t g_cmaccontext = {0};

stse_ReturnCode_t stse_platform_aes_cmac_init(const PLAT_UI32 key_idx, PLAT_UI16 exp_tag_size)
{
	g_cmaccontext.key_id = (psa_key_id_t)key_idx;
	g_cmaccontext.op = psa_mac_operation_init();
	psa_status_t status =
		psa_mac_sign_setup(&g_cmaccontext.op, g_cmaccontext.key_id, PSA_ALG_CMAC);

	LOG_DBG("AES-CMAC init with key %u, expected tag size %u: %s", key_idx, exp_tag_size,
		(status == PSA_SUCCESS) ? "success" : "failure");
	return (status == PSA_SUCCESS) ? STSE_OK : STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
}

stse_ReturnCode_t stse_platform_aes_cmac_append(PLAT_UI8 *pInput, PLAT_UI16 length)
{
	psa_status_t status = psa_mac_update(&g_cmaccontext.op, pInput, length);
	LOG_DBG("AES-CMAC update with %u bytes: %s", length,
		(status == PSA_SUCCESS) ? "success" : "failure");
	return (status == PSA_SUCCESS) ? STSE_OK : STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
}

stse_ReturnCode_t stse_platform_aes_cmac_compute_finish(PLAT_UI8 *pTag, PLAT_UI8 *pTagLen)
{
	uint8_t full_tag[16];
	size_t full_len = 0;

	psa_status_t st =
		psa_mac_sign_finish(&g_cmaccontext.op, full_tag, sizeof(full_tag), &full_len);
	psa_mac_abort(&g_cmaccontext.op);
	if (st != PSA_SUCCESS || full_len != 16) {
		LOG_ERR("AES-CMAC compute finish failed: %s",
			(st == PSA_SUCCESS) ? "invalid tag length" : "failure");
		return STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
	}

	memcpy(pTag, full_tag, 4);
	*pTagLen = 4;

	LOG_DBG("AES-CMAC compute finish successful, output length: %u", *pTagLen);
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cmac_verify_finish(PLAT_UI8 *pTag)
{
	uint8_t full_tag[16];
	size_t full_len = 0;

	psa_status_t st =
		psa_mac_sign_finish(&g_cmaccontext.op, full_tag, sizeof(full_tag), &full_len);
	psa_mac_abort(&g_cmaccontext.op);
	if (st != PSA_SUCCESS || full_len != 16) {
		LOG_ERR("AES-CMAC verify finish failed: %s",
			(st == PSA_SUCCESS) ? "invalid tag length" : "failure");
		return STSE_PLATFORM_AES_CMAC_VERIFY_ERROR;
	}

	if (memcmp(pTag, full_tag, 4) != 0) {
		LOG_ERR("AES-CMAC verify finish failed: tag mismatch");
		return STSE_PLATFORM_AES_CMAC_VERIFY_ERROR;
	}

	LOG_DBG("AES-CMAC verify finish successful");
	return STSE_OK;
}
