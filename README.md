# ST - STSELib Module for Zephyr OS

This is a Zephyr module that automatically imports the [ST STSELib](https://github.com/STMicroelectronics/STSELib) library.

> [!note]
>
> This repository is licensed under [Apache-2.0](LICENSE) to remain consistent with the rest of the 6TRON libraries.  
> The integrated **STSELib** itself is linked through the [v1.1.3 release](https://github.com/STMicroelectronics/STSELib/releases/tag/v1.1.3), pinned by commit [`562f3ef`](https://github.com/STMicroelectronics/STSELib/commit/562f3ef804dc3687b5cb4a0ab0a9e26fe7ecd6a6) in the [`west.yml`](west.yml).

## Options
The following Kconfig options are available:
- `CONFIG_LIB_STSELIB`: Enable STSELib support.
- `CONFIG_STSAFE_A120` or `CONFIG_STSAFE_A110`: Select the specific STSafe chip you are using.

Options can be set in your project's `prj.conf` file or through `menuconfig` (`west build -t menuconfig`).
```
menuconfig
  ├─ modules
  │  └─ st-stsafe-a1xx
  │     ST STSafe A1xx  --->
  │        [*] Enable ST STSELib support
  │           STSAFE Chip (STSAFE A120)  --->
  └─ ...
```


## Usage

Depending on the platform you are targeting with the STSafe-A1xx, you will need to provide C source files implementing platform-specific cryptographic functions. Common examples include:

- `cmac.c`: host-side AES-CMAC
- `ecc.c`: host-side ECC verification

These files must provide the following functions:

```c
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

Once implemented, simply add:

```c
#include "stselib.h"
```

to any source file that uses STSafe functionalities.

An example implementation is available here:  
[https://github.com/catie-aq/zephyr_zest_security_secure-element_tester](https://github.com/catie-aq/zephyr_zest_security_secure-element_tester)
