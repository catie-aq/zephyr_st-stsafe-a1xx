/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"
#include <psa/crypto.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(stsafe, CONFIG_STSAFE_LOG_LEVEL);

stse_ReturnCode_t stse_platform_aes_ecb_enc(const PLAT_UI8 *pPlaintext, PLAT_UI16 plaintext_length,
					    const PLAT_UI32 key_idx, PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 *pEncryptedtext_length)
{
	if (!pPlaintext || !pEncryptedtext) {
		LOG_ERR("Invalid parameter: plaintext or encryptedtext is NULL");
		return STSE_CORE_INVALID_PARAMETER;
	}
	if (plaintext_length != 16) {
		LOG_ERR("Invalid plaintext length: %u (must be 16)", plaintext_length);
		return STSE_CORE_INVALID_PARAMETER;
	}

	uint8_t iv[16] = {0};

	LOG_DBG("Encrypting 16 bytes with AES-ECB using key index %u", key_idx);
	return stse_platform_aes_cbc_enc(pPlaintext, plaintext_length, iv, key_idx, pEncryptedtext,
					 pEncryptedtext_length);
}

stse_ReturnCode_t stse_platform_aes_cbc_enc(const PLAT_UI8 *pPlaintext, PLAT_UI16 plaintext_length,
					    PLAT_UI8 *pInitial_value, const PLAT_UI32 key_idx,
					    PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 *pEncryptedtext_length)
{
	psa_status_t st;
	if (!pPlaintext || !pInitial_value || !pEncryptedtext) {
		LOG_ERR("Invalid parameter: plaintext, initial value or encryptedtext is NULL");
		return STSE_CORE_INVALID_PARAMETER;
	}
	if ((plaintext_length == 0) || ((plaintext_length % 16) != 0)) {
		LOG_ERR("Invalid plaintext length: %u (must be multiple of 16)", plaintext_length);
		return STSE_CORE_INVALID_PARAMETER;
	}

	psa_key_id_t key_id = (psa_key_id_t)key_idx;
	psa_cipher_operation_t op = PSA_CIPHER_OPERATION_INIT;
	size_t out_len = 0;
	size_t total = 0;

	st = psa_cipher_encrypt_setup(&op, key_id, PSA_ALG_CBC_NO_PADDING);

	if (st == PSA_SUCCESS) {
		st = psa_cipher_set_iv(&op, pInitial_value, 16);
	}

	if (st == PSA_SUCCESS) {
		st = psa_cipher_update(&op, pPlaintext, plaintext_length, pEncryptedtext,
				       plaintext_length, &out_len);
		total += out_len;
	}

	if (st == PSA_SUCCESS) {
		st = psa_cipher_finish(&op, pEncryptedtext + total, plaintext_length - total,
				       &out_len);
		total += out_len;
	}

	psa_cipher_abort(&op);

	if (st != PSA_SUCCESS) {
		LOG_ERR("Invalid parameter: plaintext, initial value or encryptedtext is NULL");
		return STSE_SESSION_ERROR;
	}

	if (pEncryptedtext_length) {
		*pEncryptedtext_length = (PLAT_UI16)total;
	}
	LOG_DBG("AES-CBC encryption successful, output length: %u", total);
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cbc_dec(const PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 encryptedtext_length,
					    PLAT_UI8 *pInitial_value, const PLAT_UI32 key_idx,
					    PLAT_UI8 *pPlaintext, PLAT_UI16 *pPlaintext_length)
{
	psa_status_t st;
	if (!pEncryptedtext || !pInitial_value || !pPlaintext) {
		LOG_ERR("Invalid parameter: encryptedtext, initial value or plaintext "
			"is NULL");
		return STSE_CORE_INVALID_PARAMETER;
	}
	if ((encryptedtext_length == 0) || ((encryptedtext_length % 16) != 0)) {
		LOG_ERR("Invalid encrypted text length: %u (must be multiple of 16)",
			encryptedtext_length);
		return STSE_CORE_INVALID_PARAMETER;
	}

	psa_key_id_t key_id = (psa_key_id_t)key_idx;
	psa_cipher_operation_t op = PSA_CIPHER_OPERATION_INIT;
	size_t out_len = 0;
	size_t total = 0;

	st = psa_cipher_decrypt_setup(&op, key_id, PSA_ALG_CBC_NO_PADDING);

	if (st == PSA_SUCCESS) {
		st = psa_cipher_set_iv(&op, pInitial_value, 16);
	}

	if (st == PSA_SUCCESS) {
		st = psa_cipher_update(&op, pEncryptedtext, encryptedtext_length, pPlaintext,
				       encryptedtext_length, &out_len);
		total += out_len;
	}

	if (st == PSA_SUCCESS) {
		st = psa_cipher_finish(&op, pPlaintext + total, encryptedtext_length - total,
				       &out_len);
		total += out_len;
	}

	psa_cipher_abort(&op);

	if (st != PSA_SUCCESS) {
		LOG_ERR("aes_cbc_dec: AES-CBC decryption failed");
		return STSE_SESSION_ERROR;
	}

	if (pPlaintext_length) {
		*pPlaintext_length = (PLAT_UI16)total;
	}
	LOG_DBG("AES-CBC decryption successful, output length: %u", total);
	return STSE_OK;
}
