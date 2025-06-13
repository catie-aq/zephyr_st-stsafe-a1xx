# ST - STSELib Module for Zephyr OS

This is a Zephyr Module to automatically import the [ST STSELib](https://github.com/STMicroelectronics/STSELib) library.

> [!IMPORTANT]
>
> This repository is licensed Apache-2 to be consistent with the rest of 6TRON libraries, but **STSELib is licensed [BSD-3](https://github.com/STMicroelectronics/STSELib/blob/main/LICENSE.txt)**.

If you're already using the [6TRON Zephyr workspace manifest](https://github.com/catie-aq/6tron_zephyr-workspace), add the following in the `west.yml` and then do a `west update`:

``````
    - name: stselib-module
      remote: catie-6tron
      repo-path: zephyr_stselib-module
      revision: main
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
