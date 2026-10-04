# Origins and acknowledgements

This beacon is based on **Dennis Real's** open-source ARDF firmware:

| Target | Repository | Pinned revision |
| --- | --- | --- |
| V1 | https://github.com/reald/uv-k5-firmware-custom | `5955ccfc8732f4a16b628276ed5fa98f2db54e55` |
| V3 | https://github.com/reald/uv-k1-k5v3-firmware-custom | `97b1890bed9628f625d787bfa42683f2da912614` |

The V1 base commit dates from 2 August 2026 and introduces negative ARDF gain
settings. V3 was pinned from reald's main branch on 4 October 2026.

The V1 lineage includes Egzumer and DualTachyon. The V3 lineage includes Armels
F4HWN port, muzkr, Egzumer, and DualTachyon. The original driver, UI, and firmware
copyright notices remain in the modified files. Both upstreams carry the
Apache-2.0 license, included here as LICENSE.

The overlay contains only added/changed files. Unchanged sources are retrieved
from the pinned upstream during preparation. Shared beacon logic is maintained
in source/app/beacon.c and source/app/beacon.h; source-v3 contains the V3 adapter.

Naming an upstream author does not imply their endorsement or a guarantee for
this experimental transmitting extension.
