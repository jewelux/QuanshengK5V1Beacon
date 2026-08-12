# Änderungsprotokoll

## Erste veröffentlichbare Beacon-Version

- eigenständiger 70-cm-Peilsenderbetrieb für UV-K5 V1
- PTT-gesteuerter Start und sofortiger Stopp mit PTT oder EXIT
- zweimalige Morsekennung und anschließend fünf Sekunden Sendepause
- 750 ms HF-Vorlauf
- Auswahl von MO sowie MOE bis MO5
- einstellbare Frequenz, relative LOW-Leistung und Tonfrequenz
- Speicherung der Beacon-Einstellungen im EEPROM
- Admin-Modus auf Basis von Dennis' originalem Menüsystem
- nicht blockierende Zustandsmaschine innerhalb der originalen Hauptschleife
- LOW-DAC bei 100 % ohne zusätzliche Division durch fünf
- Firmware-Packprogramm funktioniert auch ohne externes Python-Modul `crcmod`

Noch nicht enthalten ist die automatische Zeitsteuerung eines klassischen
Fünf-Sender-ARDF-Wettbewerbs.
