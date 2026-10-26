# Optional achievement memory extension, version 1

Copyright (C) 2026 CorvusSoftware. SPDX-License-Identifier: MIT.

The existing CorvusAPI0 ABI has no memory inspection operation. Reading battery
data or decoding private save-state layouts is not an interchangeable substitute
for reading the live address space required by RetroAchievements.

`include/achievements.h` defines a separate optional C entrypoint:
`corvus_get_achievements_api(1, out)`. The original header and its structures are
unchanged. Existing cores and hosts continue to negotiate ABI0 byte-for-byte.
Hosts must test for the extension symbol. Its absence disables achievements for
that module, with an explanation; it must never be treated as a memory of zeroes.

The output identifies the RetroAchievements console and supplies a read-only
copy callback accepting the same opaque handle created by the base API. The
frontend owns all returned bytes and evaluates achievements on the same thread
as the core, between frames. Reads neither advance nor modify emulation.

Examples for the current CorvusCore provider:

| Console | ID | RetroAchievements address space |
| --- | --- | --- |
| Game Boy | 4 | CPU-visible 0x0000-0xFFFF, with unmapped holes returning short reads |
| Game Boy Color | 6 | GB view plus extra WRAM at 0x10000-0x15FFF and cartridge RAM banks from 0x16000 |
| Game Boy Advance | 5 | IWRAM 0x0000-0x7FFF, EWRAM 0x8000-0x47FFF, optional save memory 0x48000-0x57FFF |

The provider already has side-effect-free reads for these spaces. Its CorvusAPI
adapter must expose them through this extension; a frontend must never cast a
CorvusAPI handle to that provider's private bridge handle.

Read results are exact contiguous-prefix lengths. Stop at unavailable memory or
32-bit address overflow. Consumers propagate short reads to rcheevos, allowing
unsupported conditions to be reported instead of silently awarding achievements.

Providers must rebuild and release modules exposing this extension before a
frontend advertises achievements for them. Existing ABI0 modules without the
optional symbol remain compatible and playable, with achievements unavailable.
