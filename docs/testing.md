# Verification and first hardware tests

Status: 4 October 2026. The corrected images are awaiting repeat hardware tests.

## Operator report and follow-up fix

The operator tested the images from commit `7cde3b3b8d92989a968921461332efc6c32daf28`:

| Target | Observations |
| --- | --- |
| V1 | Flashing and menu worked. One initial correct Morse transmission was heard; later transmissions had no audible tone and appeared to retain RF. |
| V3 | Flashing and Morse worked. Bottom menu instructions were clipped, and navigation/power entry was unclear. |

The real-font host test reproduced a write beyond the seven-page framebuffer:
`UI_PrintString` writes two pages but was called on page 6. This affected both
beacon status screens and the V3 admin footer. In the V1 image, settings follow
the framebuffer in RAM, so the overflow could corrupt radio settings. This is a
confirmed memory error, not proof that every possible RF problem is resolved.

The fix uses a one-page footer, lists all V3 settings with a selection arrow,
provides visible save/cancel and invalid-input messages, and isolates V1 user-mode
beacon RF control from normal receiver/TOT/power-save processing. V1's beacon
screen is explicitly flushed to the LCD. V1 admin processing is retained.
The corrected images need an operator retest, especially multiple consecutive
Morse repetitions and V3 saving/reloading of a power percentage.

## Software checks completed

ARM GNU Toolchain 12.2.1:

| Target | Code + initialized data | BSS | Distributed image |
| --- | ---: | ---: | ---: |
| V1 | 50,276 bytes | 2,684 bytes | 50,294 bytes, packed with metadata and CRC |
| V3 | 28,588 bytes | 6,272 bytes | 28,588 bytes, raw |

Both fit their linker regions. V3 RAM usage including 36 bytes of initialized data and a 1,540-byte reserved
heap/stack region is 7,848 bytes of 16 KiB. This is link-time allocation, not
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
  and percentage scaling from original LOW calibration. Both targets run ten
  consecutive automatic RF-off/restart cycles in the simulation.
- Real upstream fonts/renderers under AddressSanitizer and UndefinedBehaviorSanitizer
  check all identifiers/states and V3 browse/edit screens, including maximum values.

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

Run the runtime command above without `-DBEACON_PLATFORM_V3` and with
`-I/tmp/beacon-v1` to test the V1 adapter as well.

For display bounds checks with the actual upstream fonts/renderers:

```sh
gcc -std=gnu2x -DENABLE_BEACON_MO -DBEACON_PLATFORM_V3 -I/tmp/beacon-v3/App \
  -fsanitize=address,undefined -g -ffunction-sections -fdata-sections \
  tests/beacon-display.c /tmp/beacon-v3/App/ui/helper.c \
  /tmp/beacon-v3/App/font.c /tmp/beacon-v3/App/external/printf/printf.c \
  -Wl,--gc-sections -o /tmp/beacon-display-v3
ASAN_OPTIONS=detect_leaks=0 /tmp/beacon-display-v3
```

For V1, omit the V3 definition and use the corresponding prepared V1 paths.
Leak checking is disabled because this test allocates no heap memory and some
containers block LeakSanitizer's process inspection. Address/undefined-behavior
checks remain enabled. Optional output paths export PBM screenshots for inspection.

A successful test exits with status 0. Runtime tests simulate driver behavior;
only the display tests use actual upstream rendering code.

## Receiver setting for every field test

Both beacon builds transmit **FM**, not AM. Set the receiving radio to **FM on
the beacon frequency**. A clean tone on an FM receiver is the expected result.
AM-only peiling receivers are not directly compatible; see
[the modulation explanation](../README.md#modulation-and-receiver-compatibility).
Do not record reception alone as validation of an AM emission.

The operator subsequently reported clean FM reception on V3, and a near-field
trial at 1% LOW with sensitivity 00 and a short rubber antenna without full-scale
field-strength indication. This is encouraging for a practical range test, but
it is not a complete range or full acceptance result for both hardware targets.

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
