# Firmware flashen

## Voraussetzungen

- Quansheng UV-K5 **V1** mit DP32G030-Prozessor
- passendes serielles Programmierkabel, meist mit CH340-Chip
- installierter CH340-Treiber
- UVTools oder ein kompatibles UV-K5-Flashwerkzeug
- Sicherung von EEPROM und Kalibrierdaten

## Ablauf

1. Funkgerät ausschalten.
2. Programmierkabel vollständig einstecken.
3. PTT gedrückt halten und das Gerät einschalten.
4. In UVTools den COM-Port des Programmierkabels auswählen.
5. `release/Quansheng-K5-V1-70cm-Beacon.packed.bin` auswählen.
6. Flashvorgang starten und die Stromversorgung nicht unterbrechen.
7. Gerät nach erfolgreichem Abschluss aus- und normal wieder einschalten.

Die Datei ist nicht für V2-, V3- oder K1-Geräte geeignet. Eine `.raw.bin`
darf nicht mit UVTools geflasht werden.
