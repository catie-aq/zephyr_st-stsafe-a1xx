# STSAFE-A1xx Multi-threaded Example

Demonstrates concurrent access to a single STSAFE instance from multiple threads using the locked `stsafe_acquire()` / `stsafe_release()` API.

## Overview

Two threads run echo round-trips against the same STSAFE device in parallel:
- Thread A sends a payload of `0xAA` bytes.
- Thread B sends a payload of `0xBB` bytes.

Each thread acquires the device, runs `stse_device_echo`, releases it, then sleeps. The per-instance mutex inside the driver serialises the I²C traffic, and the payload check (sent vs received) confirms that neither thread sees the other's reply.

## Build and Flash

```bash
west build -b zest_core_nrf5340/nrf5340/cpuapp/ns \
           --shield zest_security_secureelement \
           samples/zephyr_st-stsafe-a1xx-example \
           -- -D DTC_OVERLAY_FILE="sixtron_bus.overlay"
west flash
```

## Sample Output

```
<inf> stsafe: stsafe-a120@20: ready (A120 @ 0x20, bus_id=0)
[2026-05-21T12:46:49.431Z] [nrf] │ *** Booting Zephyr OS build v4.3.0 ***
<inf> main: ************************************************************
<inf> main:    STSAFE-A1xx multi-threaded example (acquire/release)
<inf> main: ************************************************************
<inf> main: STSAFE device ready: stsafe-a120@20
<inf> main: [A] iter 0 OK
<inf> main: [B] iter 0 OK
<inf> main: [A] iter 1 OK
<inf> main: [B] iter 1 OK
<inf> main: [A] iter 2 OK
<inf> main: [B] iter 2 OK
<inf> main: [A] iter 3 OK
<inf> main: [B] iter 3 OK
<inf> main: [A] iter 4 OK
<inf> main: [B] iter 4 OK
<inf> main: [A] iter 5 OK
<inf> main: [B] iter 5 OK
<inf> main: [A] iter 6 OK
<inf> main: [B] iter 6 OK
<inf> main: [A] iter 7 OK
<inf> main: [B] iter 7 OK
<inf> main: [A] iter 8 OK
<inf> main: [B] iter 8 OK
<inf> main: [A] iter 9 OK
<inf> main: [B] iter 9 OK
<inf> main: [A] done, 0 error(s)
<inf> main: [B] done, 0 error(s)
<inf> main: All workers finished, total errors: 0
```

## See Also
- [STSAFE-A1xx Zephyr Driver](../../)
- [Single-threaded tester](../zephyr_st-stsafe-a1xx-tester)
- [STSELib](https://github.com/STMicroelectronics/STSELib)
