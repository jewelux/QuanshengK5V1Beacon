# Quansheng UV-K5 V1 und V3 – 70-cm-Peilsender

[English documentation](README.md)

> **WICHTIG — dieser Peilsender sendet FM, nicht AM:** Das gilt für V1 und V3.
> Am Empfänger **FM auf der Senderfrequenz** einstellen. Die Firmware erzeugt
> einen getasteten Morseton auf einem FM-Träger. Sie erzeugt nicht das AM-Signal
> eines klassischen 2-m-ARDF-Fuchses. Die AM-Einstellung des Quansheng betrifft
> den Empfang und schaltet den Sender nicht auf echte AM um.

Beim klassischen ARDF ist auf 2 m AM mit Morseton (A2A, 70–80 % Modulationstiefe)
und auf 80 m ein getasteter Träger (A1A/CW) vorgesehen. Die internationalen Regeln
behandeln 3,5 und 144 MHz; unser 70-cm-FM-Projekt ist für einen entsprechend
abgestimmten Trainings-/Fuchsjagdbetrieb gedacht. Ein reiner AM-Peilempfänger
ist mit dem vorgesehenen FM-Signal nicht direkt kompatibel. Echte AM-Aussendung
ist in diesem Projekt noch nicht implementiert oder geprüft.

Während der Morsezeichenpausen bleibt der Träger eingeschaltet; nach zwei
Kennungen folgen fünf Sekunden mit ausgeschalteter HF.
Quellen und ausführliche Erklärung: [Modulation and receiver compatibility](README.md#modulation-and-receiver-compatibility).

**V3:** Eine erste Portierung auf Basis von Dennis Reals V3-Firmware ist
vorbereitet. ARM-Build und PC-Tests bestanden; der Gerätetest steht aus.
V3 zeigt jetzt alle vier Menüpunkte gleichzeitig: Freq, Power, ID und Tone.
UP/DOWN bewegt den Auswahlpfeil. Für 17 %: Power auswählen, MENU, 17 eingeben,
MENU speichert. EXIT verwirft eine Eingabe. Die korrigierten Images müssen
noch einmal am Gerät getestet werden.
Firmware: `release/Quansheng-K5-V3-70cm-Beacon.bin`.
Diese Datei ausschließlich mit einem V3-kompatiblen Flashwerkzeug verwenden.
Die nachfolgende ausführliche Anleitung beschreibt V1. Die aktuellen
Anleitungen für beide Varianten stehen in [README.md](README.md) und
[FLASHING.md](FLASHING.md).

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
- Frequenz 430,013 bis 439,987 MHz in 1-kHz-Schritten
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
- EXIT: Eingabe verwerfen / zurück
- `BcnFrq`: MENU, sechs Ziffern in kHz eingeben (z. B. `433092` für
  433,092 MHz), MENU speichert. UP/DOWN verändert um 1 kHz.
- `BcnPwr`: MENU, `1` bis `100` eingeben, MENU speichert.
  1 % ist der Standard bei fehlender oder ungültiger Beacon-Konfiguration.
  Gespeicherte Werte (z. B. 17 %) bleiben nach Aus-/Einschalten erhalten.

Unvollständige oder unzulässige Eingaben werden beim Speichern abgewiesen;
EXIT verwirft sie. Alte gültige EEPROM-Konfigurationen werden übernommen,
Frequenzen dabei auf den nächsten zulässigen 1-kHz-Wert gerundet.

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
make ENABLE_BEACON_MO=1 ENABLE_PREVENT_TX=0 ENABLE_ARDF=0 ENABLE_SPECTRUM=0 ENABLE_FMRADIO=0
```

Der Beacon-Build schaltet ARDF-Empfänger, Spektrumanzeige und UKW-Radio aus,
damit das Image in den V1-Flash passt. Erfolgreich geprüft mit ARM GCC 12.2.1.

Das Skript klont Dennis' Originalprojekt, wechselt auf den festgelegten
Basis-Commit und kopiert die Dateien aus `source/` darüber. Das Ergebnis
`firmware_uvk5_v1.packed.bin` ist das flashbare Image.

## Entwicklerprüfung

Nach dem Zusammensetzen des Quellbaums lässt sich die EEPROM-Logik auch
auf einem PC mit GCC testen (Beispiel unter Linux):

```sh
gcc -std=gnu2x -DENABLE_BEACON_MO -I/path/to/prepared-upstream \
  -ffunction-sections -fdata-sections tests/beacon-config.c \
  -Wl,--gc-sections -o /tmp/beacon-config-test
/tmp/beacon-config-test
```

Der Test prüft Standardwerte, Speicherung, ungültige Leistungswerte sowie
Migration gültiger V2/V3-Konfigurationen einschließlich aller alten Frequenzen.

## Entwicklungsstand und Ausblick

Die Tastatureingabe und EEPROM-Migration sind implementiert. Der neue V1-Build
und die EEPROM-Tests wurden softwareseitig geprüft; ein Gerätetest dieses
neuen Images steht noch aus. Der vorherige Beacon-Stand wurde vom Betreiber
praktisch getestet.

Die automatische Fünf-Fuchs-Zeitsteuerung ist noch nicht implementiert.
Die diskutierte Synchronisation über Nachbarn und CTCSS ist als
[Projektidee](docs/ideas/fox-sync.md) dokumentiert.

## Sicherheit

Die Prozentanzeige bezeichnet relative PA-DAC-Werte, nicht lineare
HF-Leistung. Die Prozentwerte dienen zur praktischen Reichweiteneinstellung. EEPROM und Kalibrierdaten vor dem Flashen sichern und
die örtlichen Vorschriften beachten.

## Lizenz und Anerkennung

Siehe [`NOTICE.md`](NOTICE.md) und [`LICENSE`](LICENSE). Die Beacon-Erweiterung
benutzt ausdrücklich Dennis' Firmwarearchitektur und sein Menüsystem; sie ist
keine offizielle Veröffentlichung oder Zusicherung von Dennis Real.
