# Änderungsprotokoll

## Admin-Tastatureingabe (zur Prüfung)

Vorbereitet am 3. Oktober 2026, dokumentiert am 4. Oktober 2026.
Build und EEPROM-Tests bestanden; Hardwaretest des neuen Images steht aus.

- Frequenz direkt mit sechs Ziffern in kHz; 1-kHz-Auflösung
- Leistung direkt mit 1–100 %; MENU speichert, EXIT verwirft
- Standardleistung 1 % bei fehlender/ungültiger Konfiguration
- Gespeicherte Leistung bleibt erhalten; EEPROM V2/V3 wird auf V4 migriert
- Alte Frequenzen werden auf den nächsten zulässigen kHz-Wert gerundet
- Build-Anleitung mit passenden V1-Optionen; CRC-Fallback auch über make

## Beacon-Version

- 70-cm-Peilsenderbetrieb für UV-K5 V1
- PTT-gesteuerter Start und sofortiger Stopp mit PTT oder EXIT
- zweimalige Morsekennung und anschließend fünf Sekunden Sendepause
- 750 ms HF-Vorlauf
- Kennungen MO und MOE bis MO5
- einstellbare Frequenz, relative LOW-Leistung und Tonfrequenz
- EEPROM-Speicherung
- Admin-Modus auf Basis von Dennis' Menüsystem
- nicht blockierende 10-ms-Zustandsmaschine
- originaler LOW-DAC-Wert bei 100 %, ohne zusätzliche Division durch fünf
- interner CRC-16/XMODEM-Fallback im Packprogramm

Noch nicht enthalten: automatische Zeitsteuerung eines klassischen
Fünf-Sender-Wettbewerbs.
