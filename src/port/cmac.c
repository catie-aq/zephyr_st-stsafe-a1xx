#include "stselib.h"

#include <psa/crypto.h>
typedef struct {
	psa_key_id_t key_id;
	psa_mac_operation_t op;
} stsafea1xx_psa_cmac_ctx_t;
static stsafea1xx_psa_cmac_ctx_t g_cmaccontext;

stse_ReturnCode_t stse_platform_aes_cmac_init(const PLAT_UI8 *pKey, PLAT_UI16 key_length,
					      PLAT_UI16 exp_tag_size)
{

	psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
	psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_MESSAGE);
	psa_set_key_algorithm(&attr, PSA_ALG_CMAC);
	psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
	psa_set_key_bits(&attr, key_length * 8);

	psa_status_t status = psa_import_key(&attr, pKey, key_length, &g_cmaccontext.key_id);
	if (status) {
		return STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
	}

	g_cmaccontext.op = psa_mac_operation_init();
	status = psa_mac_sign_setup(&g_cmaccontext.op, g_cmaccontext.key_id, PSA_ALG_CMAC);
	return (status == PSA_SUCCESS) ? STSE_OK : STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
}

stse_ReturnCode_t stse_platform_aes_cmac_append(PLAT_UI8 *pInput, PLAT_UI16 length)
{
	psa_status_t status = psa_mac_update(&g_cmaccontext.op, pInput, length);
	return (status == PSA_SUCCESS) ? STSE_OK : STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
}

stse_ReturnCode_t stse_platform_aes_cmac_compute_finish(PLAT_UI8 *pTag, PLAT_UI8 *pTagLen)
{
	uint8_t full_tag[16];
	size_t full_len = 0;

	psa_status_t st =
		psa_mac_sign_finish(&g_cmaccontext.op, full_tag, sizeof(full_tag), &full_len);

	psa_destroy_key(g_cmaccontext.key_id);

	if (st != PSA_SUCCESS || full_len != sizeof(full_tag)) {
		return STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
	}
	memcpy(pTag, full_tag, *pTagLen);

	psa_destroy_key(g_cmaccontext.key_id);
	return STSE_OK;
}

stse_ReturnCode_t stse_platform_aes_cmac_verify_finish(PLAT_UI8 *pTag)
{
	uint8_t full_tag[16];
	size_t full_len = 0;

	psa_status_t st =
		psa_mac_sign_finish(&g_cmaccontext.op, full_tag, sizeof(full_tag), &full_len);

	psa_destroy_key(g_cmaccontext.key_id);

	if (st != PSA_SUCCESS || full_len != sizeof(full_tag)) {
		return STSE_PLATFORM_AES_CMAC_COMPUTE_ERROR;
	}
	memcpy(pTag, full_tag, sizeof(full_tag));

	psa_destroy_key(g_cmaccontext.key_id);
	return STSE_OK;
}
