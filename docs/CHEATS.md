# Optional native cheat extension, version 1

Copyright (C) 2026 CorvusSoftware. SPDX-License-Identifier: MIT.

`corvus_get_cheats_api` negotiates `CORVUS_CHEATS_API_VERSION == 1` independently
of ABI 0 and the other optional extensions. The CMake package remains 0.1.0.
No emulator, platform or frontend library is required. Missing export means a
legacy module: the base interface remains usable without native cheats.

## Negotiation and ownership

Zero-initialize `corvus_cheats_api`, set its `struct_size`, and request version 1.
Unknown version, NULL output or capacity below the complete version 1 structure
returns 0 without writing any bytes, including `struct_size`. Success writes only
the current structure, sets size/version, and leaves trailing host bytes intact.
All three function pointers are mandatory and valid for the module lifetime.
Use the same live opaque base handle. Serialize calls on its owner thread between
frames. No function emits video/audio callbacks or executes a frame.

## Slots and format hints

`capabilities(core, player)` reports supported native format families for a loaded
zero-based slot. NULL/unloaded handles, unavailable slots and unsupported cheats
return0. Unknown bits are reserved and must be zero in version 1. CORE_TEXT allows
independent providers to support their own text syntax. Flags describe families,
not a promise that every device opcode is implemented: `validate` is authoritative.
No specific family is required, and formats are detected by the core from the
submitted text. The host must not translate native codes to private memory writes.

## Validation and replacement

`validate(core, player, code)` checks one entry without changing the machine or
active list, including when the host keeps that entry disabled. Each code is a
nonempty NUL-terminated UTF-8 string of at most 4096 bytes excluding NUL. One entry
may contain several device-code lines. Names and enabled flags are frontend
metadata and are not part of the ABI.

`replace(core, player, codes, count)` replaces that slot's complete active list.
There are at most 128 entries; duplicates and order are preserved. The provider
copies or decodes all borrowed input before returning, and applies effects from
the next frame. `codes == NULL` is accepted only for `count == 0`, which removes
all active entries for this slot. Every invalid input or resource failure returns 0
with the previous list and emulated state unchanged. Diagnostics may change and
are exposed through base `last_error`. Validation and replacement must accept the
same individual syntax and resource limits; an aggregate list may exceed a
provider's documented processing budget and be rejected atomically.

Operations on NULL/unloaded handles or unavailable players fail. There is no
transaction across slots. Replacing player 1's list never changes player 0's list.
Clearing codes stops future effects; it does not undo memory already modified.
Codes survive reset and state restoration, are not stored in save states, and
are cleared by unload. Every successful new load starts with no active cheats.
The host owns per-content persistence and reapplies its selected list after load.

The configurable synthetic example supports CORE_TEXT with `BYTE=HH` (uppercase
hexadecimal): each active entry writes the one-byte tone on every frame. It is
not an emulator. Its tests cover ABI bounds, input ownership, transactional
rejection, reset, clearing on unload and independent player slots. The minimal
example deliberately omits the extension to exercise legacy compatibility.
