# Zephyr Zest Security Secure Element Example
This example demonstrates how to use the Zephyr Zest Security Secure Element (STSafe-A1xx) with a focus on setting up the secure element, managing keys, and establishing secure channels.

## Usage
If there is no keys already set in your secure element and you want to seed them, set the `SHOULD_SEED_HOST_KEY_SLOT` macro to `1` in `app/src/main.c[11]`. This will seed the host key slot with a default key. If you also want to debugs frames, uncomment the `#define STSE_FRAME_DEBUG_LOG` in the `app/include/stselib.h[14]` file.

## Building and Flashing
To build and flash the application, use the following commands:
```bash
west build -b zest_core_nrf5340/nrf5340/cpuapp/ns app_workspace/zephyr_zest_security_secure-element_example/app/ --pristine

west flash
```

## Current exchanges
```mermaid
---
config:
  theme: neutral
  look: neo
---
sequenceDiagram
  participant App as Application (Zephyr)
  participant SE as Secure Element (STSafe-A1xx)
  Note over App, SE: Step 1 — Handler initialization
  App ->> App: stse_set_default_handler_value(handler)
  App ->> App: stse_init(handler) start
  App ->> SE: Query Access Conditions (command 0x14 / 0x24)
  SE -->> App: AC response
  App ->> SE: Query Access Conditions (2ⁿᵈ call)
  SE -->> App: AC response
  App ->> App: stse_init(handler) complete
  Note over App, SE: Step 2 — Query host key slot
  App ->> SE: stsafea_query_host_key()
  SE -->> App: key_presence_flag and counter
  alt Slot is empty and SHOULD_SEED = 1
    Note over App, SE: Step 3 — Provisioning host key
    App ->> SE: stse_host_key_provisioning(key_128 or key_256)
    SE -->> App: STSE_OK
    App ->> SE: stsafea_query_host_key()
    SE -->> App: key_presence_flag = 1
    opt STSAFE-A120
      Note over App, SE: Step 4 — Wrap/unwrap key generation
      App ->> SE: stsafea_generate_wrap_unwrap_key()
      SE -->> App: STSE_OK
    end
  else Slot already populated or SHOULD_SEED = 0
    App ->> App: Skip provisioning
  end
  Note over App, SE: Step 5 — Open secure session
  App ->> SE: stsafea_open_host_session(mac_key, cipher_key)
  SE -->> App: Session opened
  Note over App, SE: Step 6 — Use services
  App ->> SE: stsafea_wrap_payload(data)
  SE -->> App: wrapped_data
  App ->> SE: stse_get_device_id(cert_zone_0)
  SE -->> App: device_id
  App ->> App: Log output
  Note over App: End of program
```
