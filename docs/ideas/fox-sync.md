# Projektidee: fünf drahtlos synchronisierte Füchse

Stand: 4. Oktober 2026. **Entwurf; nicht implementiert.**
Zielhardware: ausschließlich Quansheng UV-K5 V1 mit DP32G030/BK4819.

## Ziel und Ausgangslage

Fünf Geräte sollen auf einer gemeinsamen Frequenz nacheinander senden:

| Fuchs | Kennung | Zeitschlitz im Fünf-Minuten-Zyklus |
| --- | --- | --- |
| 1 | MOE | Minute 1 |
| 2 | MOI | Minute 2 |
| 3 | MOS | Minute 3 |
| 4 | MOH | Minute 4 |
| 5 | MO5 | Minute 5 |

Die heutige Firmware wiederholt eine Kennung zweimal mit anschließender
fünfsekündiger HF-Pause. Sie enthält weder diesen Zeitplan noch eine
Synchronisation. Leistung bleibt derzeit auf den relativen LOW-DAC-Bereich
begrenzt; eine Erweiterung auf höhere Leistungsstufen ist ebenfalls offen.

Im Gelände hören sich möglicherweise nur benachbarte Füchse. Fuchs 1 soll die
Zeit vorgeben; erreichbare Nachbarn sollen diese Zeit weitergeben. Eine reine
Kette ist nur möglich, wenn jeder Fuchs seinen Vorgänger in der Sendereihenfolge
empfangen kann. Räumliche Nachbarschaft allein garantiert das nicht.

## Diskutierte Verfahren

### Morsekennung als Identifikation

MOE, MOI, MOS, MOH und MO5 identifizieren den jeweiligen Zeitschlitz. Eine
wiederholte Kennung verrät jedoch ohne besonderen Marker nicht, wie viel Zeit
seit Beginn der Minute vergangen ist. Automatische Morseerkennung und ein
eindeutiger Zeitbezug müssten neu entwickelt und geprüft werden.

### CTCSS als Synchronisationshilfe

Vorschlag: Jeder Fuchs erhält einen eigenen CTCSS-Ton zusätzlich zur hörbaren
Morsekennung. Ein einmaliger Marker an einer festgelegten Stelle der Sendeminute
liefert den Zeitbezug. Die BK4819-Basis enthält Funktionen und Register für
CTCSS-Erzeugung und -Erkennung; die Beacon-Erweiterung nutzt sie dafür noch nicht.

CTCSS-Erkennung benötigt Zeit. Die tatsächliche Verzögerung, ihre Streuung und
die Erkennung bei schwachem Empfang müssen gemessen werden. Marker dürfen nicht
mit den normalen Morsepausen oder einem normalen Trägerabfall verwechselt werden.
Ob CTCSS während der vorhandenen BK4819-Morsetonerzeugung und
TX-Stummschaltung bestehen bleibt, muss am realen Signal geprüft werden.

Zunächst hört jedes Gerät auf einen fest eingestellten Ton seines Vorgängers.
Eine spätere Erkennung beliebiger erreichbarer Füchse wäre robuster, erfordert
aber ein Verfahren zur Tonwahl bzw. Suche. Gleichzeitige Erkennung von fünf
CTCSS-Tönen wird nicht vorausgesetzt.

## Zeitsteuerung und Ausfälle

- Jedes Gerät besitzt eine lokale Uhr und berechnet seinen festen Zeitschlitz.
- Ein Empfang korrigiert die Uhr; er löst nicht unmittelbar eine neue
  vollständige Sendeminute aus. Sonst würden sich Erkennungsverzögerungen addieren.
- Fuchs 1 bleibt die maßgebliche Zeitquelle. Die anderen dürfen keine unabhängig
  abweichenden Zeitquellen bilden; Weitergabe und Korrekturregeln sind noch offen.
- Schutzabstände zwischen Sendefenstern sollen Überschneidungen vermeiden.
- Bei vorübergehendem Empfangsausfall läuft die eigene Uhr zunächst weiter.
- Nach zu langer Zeit ohne gültige Synchronisation ist Sendestopp mit weiterem
  Lauschen als mögliche Regel zu prüfen. Grenzwerte sind noch nicht festgelegt.
- Ein getrenntes Funknetz ohne Empfangsweg zu Fuchs 1 kann nicht dauerhaft
  garantiert synchron gehalten werden.

## Erster Versuch mit zwei Geräten

1. Gerät A sendet einen ausreichend langen, definierten CTCSS-Marker.
2. Gerät B erkennt ihn und protokolliert die Erkennungszeit.
3. B sendet in einem daraus berechneten Zeitfenster; kein unmittelbarer Start.
4. Verzögerung und Streuung bei verschiedenen Pegeln und wiederholten Zyklen messen.
5. Empfangsausfall, erneute Synchronisation und lokale Uhrabweichung testen.
6. Erst anschließend auf fünf Geräte und Weitergabe über Nachbarn erweitern.

Offen: geeignete Töne, Markerform und -dauer, Schutzabstände, Uhrkorrektur ohne
Zeitkreise, Startprozedur im Gelände, Ausfallregeln und Stromverbrauch im Empfang.
Das Konzept ist zunächst für Trainingsbetrieb vorgesehen. Eine Eignung für
Wettkämpfe setzt Feldtests und Prüfung der jeweiligen Wettbewerbsregeln voraus.
