# Quansheng UV-K5 V1 – 70-cm-Peilsender

> **Ausgangsbasis:** Dieses Projekt ist eine Erweiterung der Firmware von
> **Dennis Real**:
> [`reald/uv-k5-firmware-custom`](https://github.com/reald/uv-k5-firmware-custom),
> Basis-Commit `5955ccfc8732f4a16b628276ed5fa98f2db54e55`.

Dieses Repository ist bewusst klein. Es kopiert nicht Dennis' komplettes
Projekt, sondern enthält unter `source/` nur die hinzugefügten oder geänderten
Dateien. Zum Übersetzen werden sie auf den oben genannten Stand seiner
Firmware gelegt.

## Funktionen

- Quansheng UV-K5 **V1** mit DP32G030 als 70-cm-Peilsender
- F2A-artiger Morse-Ton auf einem FM-Träger
- Kennungen `MO`, `MOE`, `MOI`, `MOS`, `MOH` und `MO5`
- zweimalige Kennung, danach fünf Sekunden vollständige Sendepause
- 750 ms HF-Vorlauf vor der ersten Kennung
- Frequenz 430,0125 bis 439,9875 MHz in 12,5-kHz-Schritten
- Tonfrequenz 400 bis 1500 Hz in 50-Hz-Schritten
- Leistung 1 bis 100 % des originalen LOW-DAC-Werts in 1-%-Schritten
- Speicherung der Einstellungen im EEPROM
- nicht blockierende Zustandsmaschine in Dennis' normaler Hauptschleife

PTT startet den Sender. PTT oder EXIT beendet ihn sofort. Ein automatischer
Sendestart ist nicht enthalten.

## Admin-Modus

MENU beim Einschalten gedrückt halten. Danach:

- UP/DOWN: Menüpunkt oder Wert wählen
- MENU: öffnen beziehungsweise speichern
- EXIT: zurück

Menüpunkte: `BcnFrq`, `BcnPwr`, `BcnID` und `BcnTon`. Im Admin-Modus wird
nicht gesendet.

## Fertige Firmware

Das flashbare Image liegt hier:

`release/Quansheng-K5-V1-70cm-Beacon.packed.bin`

Es ist ausschließlich für den UV-K5 V1 bestimmt. Hinweise stehen in
[`FLASHING.md`](FLASHING.md).

## Quellcode zusammensetzen und bauen

Unter Windows das Repository wegen des alten Makefiles in einem Pfad ohne
Leerzeichen verwenden. PowerShell-Beispiel:

```powershell
.\scripts\prepare-upstream.ps1 -Destination C:\UVK5BeaconBuild
Set-Location C:\UVK5BeaconBuild
make clean
make ENABLE_BEACON_MO=1 ENABLE_PREVENT_TX=0
```

Das Skript klont Dennis' Originalprojekt, wechselt auf den festgelegten
Basis-Commit und kopiert die Dateien aus `source/` darüber. Das Ergebnis
`firmware_uvk5_v1.packed.bin` ist das flashbare Image.

## Sicherheit

Die Prozentanzeige bezeichnet relative PA-DAC-Werte, nicht lineare
HF-Leistung. Jede Einstellung vor Antennenbetrieb an einem 50-Ohm-Dummyload
mit Wattmeter prüfen. EEPROM und Kalibrierdaten vor dem Flashen sichern und
die örtlichen Vorschriften beachten.

## Lizenz und Anerkennung

Siehe [`NOTICE.md`](NOTICE.md) und [`LICENSE`](LICENSE). Die Beacon-Erweiterung
benutzt ausdrücklich Dennis' Firmwarearchitektur und sein Menüsystem; sie ist
keine offizielle Veröffentlichung oder Zusicherung von Dennis Real.
