# ST - STSELib Module for Zephyr OS

This is a Zephyr Module to automatically import the [ST STSELib](https://github.com/STMicroelectronics/STSELib) library.

> [!IMPORTANT]
>
> This repository is licensed Apache-2 to be consistent with the rest of 6TRON libraries, but **STSELib is licensed [BSD-3](https://github.com/STMicroelectronics/STSELib/blob/main/LICENSE.txt)**.

## Integration

If you're already using the [6TRON Zephyr workspace manifest](https://github.com/catie-aq/6tron_zephyr-workspace), add the following in the `west.yml` (workspace -> 6tron-project) and then do a `west update`:

``````
    - name: stselib-module
      remote: catie-6tron
      repo-path: zephyr_stselib-module
      revision: v1.1.1
      path: 6tron/stselib_module
      submodules: true
``````

If not, you will need to first add the external remote in your zephyr manifest (or your custom workspace manifest) before adding the previous element:

``````
manifest:
  remotes:
    - name: catie-6tron
      url-base: https://github.com/catie-aq
``````

## Usages

Depending on the platform you’re using with the STSafe-A1xx and the use case you wish to implement, you’ll need to provide C source files to handle communication. Common examples include the `cmac.c` and `ecc.c` modules, which implement host-channel encryption and signature functionality.

These two files must implement the following functions: 
```C
// cmac.c
stse_ReturnCode_t stse_platform_aes_cmac_init(const PLAT_UI8 *pKey, PLAT_UI16 key_length,
					      PLAT_UI16 exp_tag_size);
stse_ReturnCode_t stse_platform_aes_cmac_append(PLAT_UI8 *pInput, PLAT_UI16 length);
stse_ReturnCode_t stse_platform_aes_cmac_compute_finish(PLAT_UI8 *pTag, PLAT_UI8 *pTagLen);
stse_ReturnCode_t stse_platform_aes_cmac_verify_finish(PLAT_UI8 *pTag);

// ecc.c
stse_ReturnCode_t stse_platform_ecc_verify(stse_ecc_key_type_t key_type, const PLAT_UI8 *pPubKey,
					   PLAT_UI8 *pDigest, PLAT_UI16 digestLen,
					   PLAT_UI8 *pSignature);
```

Then simply add `#include "stselib.h"` to any source file where you want to use the STSafe functionalities.

An example implementation is available in this repository: https://github.com/catie-aq/zephyr_stsafe-a110-example

You can take a look at [this commit](https://github.com/catie-aq/zephyr_zest_security_secure-element_tester/commit/524f36a942f5477f8f60efb41ad82148ba56ef96) for the minimal configuration needed. 



