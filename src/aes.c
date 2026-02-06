/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stselib.h"
#include <psa/crypto.h>

static psa_status_t import_aes_key(psa_key_id_t *key_id, const uint8_t *key, size_t key_len,
				   psa_key_usage_t usage, psa_algorithm_t alg)
{
	psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
	psa_set_key_bits(&attr, key_len * 8);
	psa_set_key_usage_flags(&attr, usage);
	psa_set_key_algorithm(&attr, alg);

	psa_status_t st = psa_import_key(&attr, key, key_len, key_id);

	psa_reset_key_attributes(&attr);
	return st;
}

stse_ReturnCode_t stse_platform_aes_ecb_enc(const PLAT_UI8 *pPlaintext, PLAT_UI16 plaintext_length,
					    const PLAT_UI8 *pKey, PLAT_UI16 key_length,
					    PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 *pEncryptedtext_length)
{
	psa_status_t st;
	if (!pPlaintext || !pKey || !pEncryptedtext) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if ((plaintext_length == 0) || ((plaintext_length % 16) != 0)) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if (!((key_length == 16) || (key_length == 32))) {
		return STSE_CORE_INVALID_PARAMETER;
	}

	psa_key_id_t key_id = PSA_KEY_ID_NULL;
	st = import_aes_key(&key_id, pKey, key_length, PSA_KEY_USAGE_ENCRYPT,
			    PSA_ALG_ECB_NO_PADDING);
	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

	psa_cipher_operation_t op = PSA_CIPHER_OPERATION_INIT;
	size_t out_len = 0;
	size_t total = 0;

	st = psa_cipher_encrypt_setup(&op, key_id, PSA_ALG_ECB_NO_PADDING);

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
	psa_destroy_key(key_id);

	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

	if (pEncryptedtext_length) {
		*pEncryptedtext_length = (PLAT_UI16)total;
	}
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cbc_enc(const PLAT_UI8 *pPlaintext, PLAT_UI16 plaintext_length,
					    PLAT_UI8 *pInitial_value, const PLAT_UI8 *pKey,
					    PLAT_UI16 key_length, PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 *pEncryptedtext_length)
{
	psa_status_t st;
	if (!pPlaintext || !pInitial_value || !pKey || !pEncryptedtext) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if ((plaintext_length == 0) || ((plaintext_length % 16) != 0)) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if (!((key_length == 16) || (key_length == 32))) {
		return STSE_CORE_INVALID_PARAMETER;
	}

	psa_key_id_t key_id = PSA_KEY_ID_NULL;
	st = import_aes_key(&key_id, pKey, key_length, PSA_KEY_USAGE_ENCRYPT,
			    PSA_ALG_CBC_NO_PADDING);
	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

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
	psa_destroy_key(key_id);

	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

	if (pEncryptedtext_length) {
		*pEncryptedtext_length = (PLAT_UI16)total;
	}
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cbc_dec(const PLAT_UI8 *pEncryptedtext,
					    PLAT_UI16 encryptedtext_length,
					    PLAT_UI8 *pInitial_value, const PLAT_UI8 *pKey,
					    PLAT_UI16 key_length, PLAT_UI8 *pPlaintext,
					    PLAT_UI16 *pPlaintext_length)
{
	psa_status_t st;
	if (!pEncryptedtext || !pInitial_value || !pKey || !pPlaintext) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if ((encryptedtext_length == 0) || ((encryptedtext_length % 16) != 0)) {
		return STSE_CORE_INVALID_PARAMETER;
	}
	if (!((key_length == 16) || (key_length == 32))) {
		return STSE_CORE_INVALID_PARAMETER;
	}

	psa_key_id_t key_id = PSA_KEY_ID_NULL;
	st = import_aes_key(&key_id, pKey, key_length, PSA_KEY_USAGE_DECRYPT,
			    PSA_ALG_CBC_NO_PADDING);
	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

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
	psa_destroy_key(key_id);

	if (st != PSA_SUCCESS) {
		return STSE_SESSION_ERROR;
	}

	if (pPlaintext_length) {
		*pPlaintext_length = (PLAT_UI16)total;
	}
	return STSE_OK;
}
