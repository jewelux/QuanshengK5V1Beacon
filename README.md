# Quansheng UV-K5 V1 – 70-cm-ARDF-Peilsender

> **Ausgangsbasis und Anerkennung:** Dieses Projekt basiert auf der Firmware von
> **Dennis Real** und seinem Repository
> [`reald/uv-k5-firmware-custom`](https://github.com/reald/uv-k5-firmware-custom).
> Ausgangspunkt der Entwicklung war Commit
> `5955ccfc8732f4a16b628276ed5fa98f2db54e55` vom 2. August 2026.
> Die ursprüngliche Git-Historie wurde bewusst beibehalten.

Experimentelle Spezialfirmware, die einen **Quansheng UV-K5 V1 mit
DP32G030-Prozessor** als 70-cm-Peilsender betreibt. Sie ist keine allgemeine
Funkgerätefirmware und darf nicht auf V2-, V3- oder K1-Hardware geflasht werden.

## Funktionen

- F2A-artige Aussendung: hörbarer Morse-Ton über einen FM-Träger
- Kennungen `MO`, `MOE`, `MOI`, `MOS`, `MOH` und `MO5`
- Zyklus: Kennung zweimal, danach fünf Sekunden vollständige Sendepause
- 750 ms HF-Vorlauf vor der ersten Kennung zur Stabilisierung des Senders
- Sendefrequenz 430,0125 bis 439,9875 MHz in 12,5-kHz-Schritten
- Tonfrequenz 400 bis 1500 Hz in 50-Hz-Schritten
- Leistung 1 bis 100 % des originalen LOW-DAC-Werts in 1-%-Schritten
- Einstellungen werden im EEPROM gespeichert
- nicht blockierende 10-ms-Zustandsmaschine
- Integration in Dennis' Hauptschleife, Tastaturentprellung und Menüsystem

Die Firmware sendet nicht automatisch. Im normalen Betrieb startet PTT den
Beacon; PTT oder EXIT beendet ihn sofort.

## Admin-Modus

1. Gerät ausschalten.
2. MENU gedrückt halten und einschalten.
3. MENU erst nach der Aufforderung loslassen.
4. Mit UP/DOWN den Menüpunkt wählen und mit MENU öffnen.
5. Wert mit UP/DOWN ändern; MENU speichert, EXIT geht zurück.

Menüpunkte: `BcnFrq`, `BcnPwr`, `BcnID` und `BcnTon`. Im Admin-Modus wird
nicht gesendet. Änderungen gelten beim nächsten normalen Einschalten.

Standardwerte: 433,5000 MHz, 20 % LOW-DAC, `MO`, 1000 Hz und 12 WPM.

## Fertiges Image

Zum Flashen liegt unter `release/` ein gepacktes Image. Ausschließlich die
Datei mit der Endung `.packed.bin` verwenden. Das Image ist nur für den
UV-K5 V1 bestimmt.

## Bauen

Benötigt werden GNU Make, eine ARM-none-eabi-GCC-Toolchain und Python 3.
Unter Windows sollte das Repository wegen einer Einschränkung des übernommenen
Makefiles in einem Pfad **ohne Leerzeichen** liegen.

```sh
make clean
make ENABLE_BEACON_MO=1 ENABLE_PREVENT_TX=0
```

Die Build-Ausgabe `firmware_uvk5_v1.packed.bin` ist das flashbare Image.
Weitere Details der ursprünglichen Firmware stehen in
[`UPSTREAM_README.md`](UPSTREAM_README.md).

## Sicherheit und Recht

Die Prozentanzeige bezeichnet relative PA-DAC-Werte und keine lineare
HF-Leistung. Vor Antennenbetrieb muss jede Einstellung an einem
50-Ohm-Dummyload mit einem geeigneten Wattmeter geprüft werden. Frequenz,
Leistung, Kennung, Einschaltdauer und automatischer Betrieb müssen den lokalen
Vorschriften entsprechen. EEPROM und Kalibrierdaten vor dem Flashen sichern.

## Lizenz und Herkunft

Das Projekt steht wie die Ausgangsfirmware unter der Apache License 2.0.
Siehe [`LICENSE`](LICENSE) und [`NOTICE.md`](NOTICE.md). Änderungen gegenüber
der Ausgangsbasis sind in [`CHANGELOG.md`](CHANGELOG.md) zusammengefasst.
