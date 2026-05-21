# STSAFE-A1xx Tester Sample

Minimal smoke-test for the [`st-stsafe-a1xx`](../../) Zephyr driver. Uses the single-threaded `stsafe_get_handle()` API.

## Overview

This sample:
1. Validates hardware (I²C, reset GPIO, boot sequence).
2. Sends an 8-byte echo (`stse_device_echo`).
3. Prints chip personalization info (access conditions, encryption flags).
4. Queries the host key slot state (A110/A120).

## Build and Flash

Targeting the nRF5340 app core with the `zest_security_secureelement` shield (port 1):

```bash
west build -b zest_core_nrf5340/nrf5340/cpuapp/ns \
           --shield zest_security_secureelement \
           samples/zephyr_st-stsafe-a1xx-tester \
           -- -D DTC_OVERLAY_FILE=sixtron_bus.overlay
west flash
```

> Note: For custom wiring, provide your own overlay defining the stsafe_1_20 node.

## Output Guide
- AC (Access Condition): FREE, HOST, ADMIN, or NEVER.
- CMD_enc / RSP_enc: Payload encryption required (Y/N).
- Presence Flag: 1 if a host key is provisioned. If 0, HOST commands will fail.
- C-MAC Counter: Host-session protocol counter.

## Troubleshooting

| Symptom                           | Cause / Fix                                                    |
|-----------------------------------|----------------------------------------------------------------|
| `STSAFE device not ready`         | Missing `stsafe_1_20` DT node. Check shield/overlay.           |
| `Could not acquire STSAFE handle` | I²C/GPIO error. Enable CONFIG_STSAFE_LOG_LEVEL_DBG=y to trace. |
| `stse_device_echo failed`         | The echo command is locked (not FREE) on this specific chip.   |
| Wrong host-key branch             | DT compatible mismatch (`st,stsafe-a110` vs`st,stsafe-a120`).  |

## See Also
- [STSAFE-A1xx Zephyr Driver](../../)
- [Multi-threaded example](../zephyr_st-stsafe-a1xx-example)
- [STSELib](https://github.com/STMicroelectronics/STSELib)
