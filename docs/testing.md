# Verification and first hardware tests

Status: 4 October 2026. Both current images are awaiting hardware tests.

## Software checks completed

ARM GNU Toolchain 12.2.1:

| Target | Code + initialized data | BSS | Distributed image |
| --- | ---: | ---: | ---: |
| V1 | 50,308 bytes | 2,684 bytes | 50,326 bytes, packed with metadata and CRC |
| V3 | 28,332 bytes | 6,268 bytes | 28,332 bytes, raw |

Both fit their linker regions. V3 RAM usage including 36 bytes of initialized data and a 1,536-byte reserved
heap/stack region is 7,840 bytes of 16 KiB. This is link-time allocation, not
measured peak stack.
SHA-256 checksums are in [release/SHA256SUMS](../release/SHA256SUMS).
The V1 packed CRC is independently checked with Python's CRC-16/XMODEM function.
The V3 vector table is checked for a RAM stack pointer and Thumb reset address
inside its linked application region starting at 0x08002800.

Host checks cover:

- Blank/corrupt configuration defaults to 1%; valid settings survive reload.
- Frequency/power range rejection; migration of all 799 legacy grid frequencies.
- Legacy beacon format 2 power settings and format 3 persistence.
- V3 writes use 0x00E000 with sector preservation, rather than channel storage.
- V3 keypad rejects incomplete frequencies, 0%, 101%, and out-of-range frequency.
- EXIT cancels; UP/DOWN adjusts; PTT is consumed in admin mode.
- Complete two-word MO5 sequence fits the event buffer.
- Runtime simulation verifies 75 warm-up ticks, two-word sequence, 500 RF-off
  ticks, restart, press/release/held handling, PTT/EXIT stop, battery interlock,
  and percentage scaling from original LOW calibration.

These tests simulate drivers; they do not establish actual frequency, modulation,
RF-off leakage, flash reliability, or transmission range.

## Run host checks

Prepare both upstream source trees as described in README.md. From this repo:

```sh
gcc -std=gnu2x -DENABLE_BEACON_MO -I/tmp/beacon-v1 \
  -ffunction-sections -fdata-sections tests/beacon-config.c \
  -Wl,--gc-sections -o /tmp/beacon-v1-test
/tmp/beacon-v1-test

gcc -std=gnu2x -DENABLE_BEACON_MO -DBEACON_PLATFORM_V3 -I/tmp/beacon-v3/App \
  -ffunction-sections -fdata-sections tests/beacon-config.c \
  -Wl,--gc-sections -o /tmp/beacon-v3-test
/tmp/beacon-v3-test

gcc -std=gnu2x -DENABLE_BEACON_MO -DBEACON_PLATFORM_V3 -I/tmp/beacon-v3/App \
  -ffunction-sections -fdata-sections tests/beacon-runtime.c \
  -Wl,--gc-sections -o /tmp/beacon-runtime-test
/tmp/beacon-runtime-test
```

A successful test exits with status 0. Runtime simulation covers the shared state
machine against V3 adapter stubs; it is not a V1 RF-driver simulation.

## Hardware acceptance — perform separately for each target

1. Record exact hardware model, current firmware, and backup files.
2. Flash the matching image. Normal startup must not transmit.
3. Hold MENU at startup. Confirm all four settings; PTT must not transmit here.
4. Save frequency 433092, power 17%, chosen identifier and tone. Restart and
   confirm every setting remains stored.
5. Try an incomplete frequency, 439988, power 0 and power 101. Confirm rejection;
   use EXIT and confirm the saved value remains unchanged.
6. Start at a small power setting and listen on a separate receiver. Check
   frequency, audible Morse, two identifiers, five-second RF-off pause, and repeat.
7. Test all identifiers, particularly MO5, and several tone settings.
8. Press PTT again; repeat using EXIT. Verify prompt stop including during warm-up
   and during the five-second pause. Restart and confirm TX remains off.
9. Check retained channels and calibration. For V1 reserve slot 200; V3 beacon
   settings must not alter any normal channel.
10. Test practical reception range. Increase the percentage until it meets the
    intended field layout; record a useful setting per radio. Precise watt
    calibration is not required for this project goal.

Record observed behavior and problems in the pull request before approving the
current images for routine use. Automatic scheduling and CTCSS synchronization
are outside this acceptance test because they have not been implemented.
