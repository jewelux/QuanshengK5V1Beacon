# Proposal: five wirelessly synchronized fox transmitters

Updated 4 October 2026. **Design draft; not implemented.**
Target platforms: Quansheng UV-K5 V1 and V3. Each requires its own firmware build.

## Goal and present behavior

Five radios transmit in turn on a common frequency:

| Fox | Identifier | Slot in the five-minute cycle |
| --- | --- | --- |
| 1 | MOE | Minute 1 |
| 2 | MOI | Minute 2 |
| 3 | MOS | Minute 3 |
| 4 | MOH | Minute 4 |
| 5 | MO5 | Minute 5 |

Current beacon firmware sends the chosen identifier twice and then pauses RF for
five seconds. Neither the above schedule nor synchronization is implemented.
Power is limited to a percentage of the original LOW calibration; higher-power
operation remains a separate future extension.

In the field, only neighboring foxes may hear each other. Fox 1 would provide
the time reference; reachable neighbors would relay it. A simple chain works
only if every fox can receive its predecessor in the transmission sequence.
Physical proximity alone does not guarantee that ordering.

## Candidate methods

### Morse identification

MOE/MOI/MOS/MOH/MO5 identify the transmitting slot, but a repeated identifier
alone does not reveal time elapsed since the start of the minute. Automatic
Morse recognition and a unique timing reference require new implementation
and tests.

### CTCSS timing assistance

Proposal: assign each fox its own CTCSS tone in addition to audible Morse.
A one-off marker at a defined point in the transmitting minute provides timing.
The radio driver has CTCSS generation/detection support; the beacon does not yet
use it for synchronization.

CTCSS detection takes time. Measure latency, jitter, and weak-signal behavior.
The marker must be distinguishable from Morse pauses and ordinary carrier loss.
Confirm whether CTCSS remains present during the existing Morse tone generation
and TX mute operations, separately on V1 and V3 hardware.

Initially each radio listens for a fixed predecessor tone. Later recognition of
any reachable fox would be more robust but needs a tone-selection/search method.
Simultaneous decoding of five tones is not assumed.

## Scheduling and failures

- Each radio keeps a local clock and calculates its fixed slot.
- Reception corrects the clock rather than immediately triggering a full minute
  of transmission, which would accumulate detection delays.
- Fox 1 remains the authoritative time source. Relays must not introduce
  independent time references or correction loops; rules remain to be designed.
- Guard intervals between slots help prevent overlapping transmissions.
- Temporary loss of reception uses clock holdover.
- Consider stopping TX and continuing to listen after prolonged loss of valid
  synchronization. Thresholds have not been selected.
- A disconnected group with no reception path to fox 1 cannot be guaranteed
  to remain synchronized indefinitely.

## First experiment: two radios

1. A sends a sufficiently long, defined CTCSS marker.
2. B detects it and records detection time.
3. B sends in a calculated time window, rather than immediately on detection.
4. Measure latency/jitter over repeated cycles and different signal levels.
5. Test signal loss, resynchronization, and clock drift.
6. Extend to five radios and neighbor relaying only after these results.

Open questions: tones, marker shape/duration, guards, loop-free clock correction,
field startup, failure rules, and receive power consumption. Test V1/V1, V3/V3,
and mixed V1/V3 pairs. The initial goal is training operation; competition use
requires field validation and checking the applicable event rules.
