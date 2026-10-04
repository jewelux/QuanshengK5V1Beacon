# Flashing V1 and V3

> **Both images transmit FM Morse audio, not AM.** Use an FM receiver on the
> beacon frequency. An AM menu setting does not provide AM transmission. See
> [modulation and receiver compatibility](README.md#modulation-and-receiver-compatibility)
> before choosing receivers for a hunt.

Identify the hardware before selecting firmware. V1 uses DP32G030; V3 uses
PY32F071. A V3 label alone is not conclusive: the upstream project reports
mislabeled V2 devices. Compare the installed version and upstream hardware
identification instructions. A software version called “V3” does not establish
the processor type.

Back up channels/settings and calibration with a tool compatible with your
current firmware **before** flashing. Keep the original backup and firmware.
Current beacon images are test builds awaiting hardware validation.

## UV-K5 V1

Use a serial programming cable and a V1-compatible tool such as UVTools.

1. Switch the radio off and fully insert the programming cable.
2. Hold PTT while switching on to enter firmware-update mode.
3. Select the correct COM port.
4. Select `release/Quansheng-K5-V1-70cm-Beacon.packed.bin`.
5. Flash without interrupting power; switch off and restart normally.

This packed image is exclusively for V1. Channel slot 200 is reserved for beacon
settings. Keep a backup if it originally contained a useful channel.

## UV-K5 V3

Follow the [pinned upstream project's flashing procedure](https://github.com/reald/uv-k1-k5v3-firmware-custom/tree/97b1890bed9628f625d787bfa42683f2da912614#flashing-the-firmware-with-uvtools2)
using a V3-compatible tool, such as [UVTools2](https://armel.github.io/uvtools2/).
Its WebSerial interface needs a supported browser, typically Chrome or Edge.

1. Switch off and fully insert the serial programming cable.
2. Hold PTT while switching on to enter firmware-update mode.
3. Select `release/Quansheng-K5-V3-70cm-Beacon.bin` and the correct serial port.
4. Complete the flash without interrupting power; restart normally.

V3 uses a raw `.bin`, not the V1 packed file. The beacon settings use external
flash sector 0x00E000; keep a full external-flash backup where available. The
normal channels and calibration retain their upstream locations.

## First start

Normal startup must show the beacon screen without transmitting. Hold MENU at
power-on for admin mode. Set frequency, ID, tone, and a small power percentage.
Restart, then press PTT to start. Check PTT and EXIT stop behavior before a field
range test. The full acceptance checklist is in [docs/testing.md](docs/testing.md).
