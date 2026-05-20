# Zephyr driver for ST STSAFE-A1xx secure element

Native Zephyr driver for the STMicroelectronics STSAFE-A1xx family
of secure elements, backed by the official
[ST STSELib](https://github.com/STMicroelectronics/STSELib).

This module provides:

- A proper Zephyr device (`compatible = "st,stsafe-a120"` or
`"st,stsafe-a110"`) instantiated through `DEVICE_DT_INST_DEFINE`,
multi-instance capable and multi-variant capable — an A110 and
an A120 can coexist in the same firmware on different I²C buses.
- Two usage modes: a simple mode (no locking, single-threaded
caller) and a locked mode (acquire/release with a per-instance
mutex, multi-thread safe). Each device latches into one mode on first
use; see Public API below.
- A Zephyr platform layer (I²C, GPIO reset, CRC16, crypto, optional
AES / CMAC / key store) implementing the callbacks expected by the
STSELib.
- The STSELib itself pulled in as a west dependency — pinned at v1.2.0
(commit dc93a1c) via west.yml.

> [!note]
>
> This repository is licensed under [Apache-2.0](LICENSE) consistent with
> the rest of the 6TRON libraries. The STSELib retains its own license.

## Standalone overlay

Without shield

```dts
&sixtron_i2c {
    status = "okay";
    clock-frequency = <I2C_BITRATE_STANDARD>;

    stsafe: stsafe-a120@20 {
        compatible = "st,stsafe-a120";
        reg = <0x20>;
        reset-gpios = <&gpioX Y GPIO_ACTIVE_LOW>;
        status = "okay";
    };
};
```

With shield (recommended)

```dts
ZEST_SECURITY_SECUREELEMENT(1)
```

## Samples

| Sample                                                      | Purpose                     |
|-------------------------------------------------------------|----------------------------|
| [`samples/tester`](./samples/zephyr_st-stsafe-a1xx-tester/) | Common STSAFE commands (echo, host-key query, perso info). |
| [`samples/example`](./samples/zephyr_st-stsafe-a1xx-example/) | Example of using the driver in a multi-threaded environment. |

## License
Apache-2.0 for this module. STSELib retains its own license — [see
its repository](https://github.com/STMicroelectronics/STSELib).
