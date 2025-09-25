# Zest Security Secure Element (STSafe-A1xx) Tester

This repository serves two purposes:
1. Testing your components: it verifies that everything works and displays the configured access commands as well as any pre-configured keys.
2. Minimal configuration reference: it shows the bare-bones setup required to use the Zest_Security_SecureElement.

Sample output:

## Using STSafe-A120 chip type
```
*** Booting Zephyr OS build v4.1.0 ***
************************************************************

      Zest Security Secure Element (STSafe-A1xx) Tester

************************************************************

Using STSafe-A120 chip type.

STSafe handler initialized successfully.

=== 45 commands configured (not specified commands are FREE) ===
        Cmd 08: AC=NEVER, CMD_enc=N, RSP_enc=N

Host Key Slot V2:
        Presence Flag: 0
        Key Type: 0
        C-MAC Counter: 0x00000000
```

## Using STSafe-A110 chip type

```
*** Booting Zephyr OS build v4.1.0 ***
************************************************************

      Zest Security Secure Element (STSafe-A1xx) Tester

************************************************************

Using STSafe-A110 chip type.

STSafe handler initialized successfully.

=== 9 commands configured (not specified commands are FREE) ===
        Cmd 0E: AC=HOST, CMD_enc=Y, RSP_enc=N
        Cmd 0F: AC=HOST, CMD_enc=N, RSP_enc=Y

Host Key Slot:
        Presence Flag: 1
        C-MAC Counter: 0x000041
```
