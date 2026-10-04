# Changelog

## Hardware-test follow-up — 4 October 2026

- Fix a two-page font starting on the last of seven framebuffer pages. It
  overwrote memory beyond the display buffer; the V1 linker placed settings
  immediately after that buffer. The final beacon hint now uses a one-page font.
- V1 user-mode scheduler gives the beacon exclusive RF control; admin menu
  processing is retained. Flush the V1 beacon framebuffer to the LCD explicitly.
- V3 admin lists Freq, Power, ID and Tone together with a selection arrow.
  Clear edit/save/cancel hints fit on the last two pages; invalid input has feedback.
- Regression tests use actual upstream fonts/renderers with AddressSanitizer
  and UndefinedBehaviorSanitizer, plus ten automatic repeat cycles on both targets.
- Operator reported successful V1 flashing/menu and V3 flashing/Morse on the
  previous images. Corrected images still require a repeat hardware test.

## V1/V3 project and English documentation — 4 October 2026

- English main documentation, with a German companion README.
- Separate reproducible builds pinned to reald's V1 and V3 repositories.
- Initial V3 beacon port using the shared Morse/configuration state machine.
- V3 four-item admin screen with six-digit frequency and 1–100% power input.
- V3 settings in external-flash sector 0x00E000, separate from channels/calibration.
- V3 relative power based directly on original LOW calibration.
- Dedicated V3 scheduler hooks prevent normal scan/PTT-session logic taking over.
- Shared start/stop checks prevent TX at critical battery voltage or overvoltage.
- Host tests extended for V3 flash storage, keypad validation, cancellation,
  persistence, and complete MO5 sequences.
- Both current images build successfully; hardware validation is pending.
- Five-fox synchronization remains a design proposal for both targets.

## Admin keypad entry — prepared 3 October, documented 4 October 2026

- Direct six-digit frequency entry in kHz with 1 kHz resolution.
- Direct 1–100% power entry; MENU saves and EXIT cancels.
- Default 1% for missing/invalid configuration; saved values persist.
- Beacon record versions 2/3 migrate to version 4.
- Legacy frequencies round to the nearest allowed kHz.
- V1 build options and CRC fallback integrated into Make.
- Build and host tests pass; current image hardware test pending.

## Initial V1 beacon

- PTT-controlled 70 cm beacon; PTT or EXIT stops operation.
- Two Morse identifiers followed by five seconds with RF off.
- 750 ms carrier warm-up; MO and MOE through MO5 identifiers.
- Adjustable frequency, relative LOW power, and audio tone.
- EEPROM storage and admin mode based on Dennis' menu system.
- Non-blocking 10 ms state machine.
- Original LOW DAC value at 100%, without extra division by five.
- Internal CRC-16/XMODEM fallback in the packing tool.

Automatic five-transmitter scheduling is not implemented.
