# Optional core configuration extensions, version 1

Copyright (C) 2026 CorvusSoftware. SPDX-License-Identifier: MIT.

This document specifies two independent optional extensions declared by the
canonical `<corvusapi.h>`. They add no fields to ABI 0, require no emulator or
frontend library, and do not change the CMake package version 0.1.0.

## Discovery and negotiation

Resolve `CORVUS_OPTIONS_ENTRYPOINT` (`corvus_get_options_api`) and
`CORVUS_SUBSYSTEM_ENTRYPOINT` (`corvus_get_subsystem_api`) separately from the
required `corvus_get_api`. An absent symbol is a supported legacy module: omit
that feature and continue using the base API. Do not inspect private engine
handles to replace a missing extension.

The frontend zero-initializes each API output and sets `struct_size` to its
capacity in bytes. Request `CORVUS_OPTIONS_API_VERSION == 1` or
`CORVUS_SUBSYSTEM_API_VERSION == 1`. Each entrypoint returns 1 on success and
fills `api_version`, the current `struct_size`, and its function pointers.
Unsupported versions, NULL outputs and sizes below the complete version-1
structure return 0 **without writing any output bytes**. Successful calls write
only the current structure, leaving any trailing host storage untouched.

Validate the version, size and every mandatory function before calling it.
Options requires all four functions. Subsystems requires `count`, `describe`,
`load` and `capabilities`; its four battery functions form an optional complete
group, all present or all NULL. Function pointers are valid while the module is
loaded. These rules do not add requirements to existing ABI 0 negotiations.

## Shared ownership, metadata and threading

Every instance operation takes exactly the opaque handle returned by base
`create`. NULL handles fail or return zero, except option `get_value`, which
returns `UINT32_MAX`. Other handles must be live instances from that module.
The frontend serializes operations on the owner thread between `run_frame`
calls. Only `run_frame` emits video/audio callbacks; setters, descriptions and
loads never emit them. Metadata is available on an unloaded instance immediately
after creation. Subsystem metadata requires no instance.

All metadata strings and arrays are immutable and valid for the module lifetime,
including after an instance is destroyed. Counts, ordering and array shapes are
fixed for that lifetime; selection changes, load and unload cannot change them.
Each nested structure has its version-1 `struct_size`; arrays use the native
version-1 C layout and stride. Descriptions check the caller's `struct_size` and
reject NULL, out-of-range indexes or undersized outputs without writing.

Keys, value IDs and subsystem IDs are nonempty ASCII tokens using only
`A-Z`, `a-z`, `0-9`, `_`, `.` and `-`, at most 95 bytes excluding NUL.
Option keys and subsystem IDs are unique per module; value IDs are unique within
their option. Frontends persist these tokens, never translated labels or array
indexes alone. Labels are nonempty UTF-8 and descriptions may be empty UTF-8;
both are at most 255 bytes excluding NUL. Subsystem extension strings are
nonempty, at most 255 bytes, with suffixes separated by `|` and no leading dot.
All strings are NUL-terminated. Frontends validate bounds and counts before
iterating metadata. A verifier executes trusted native code and cannot make an
invalid native pointer safe.

## Core options

`count(core)` returns 0 through 64 options, and 0 for NULL. Each
`corvus_core_option` describes a stable key, label, description, flags, a
`values` array of 1 through 64 `corvus_option_value` choices, and a valid
`default_index`. Version 1 defines only `CORVUS_OPTION_RESTART_REQUIRED == 1`.
The create operation selects every default. No choice is selected implicitly
by a frontend display or a failed setter.

`get_value(core, key)` returns the selected index. Unknown or NULL keys and NULL
handles return `UINT32_MAX`. `set_value(core, key, index)` returns 1 only for a
known key and an index below `value_count`. Unknown keys, NULL input, and invalid
indexes fail atomically, leaving all previous selections and the live session
unchanged. No video/audio callback may run during a setter.

An option with no restart flag takes effect immediately, before the next frame.
For `CORVUS_OPTION_RESTART_REQUIRED`, `get_value` reports the pending selection,
but running content keeps the previous applied value until the next successful
load or reset. Failed loads or resets do not apply pending selections. Selections
survive unload; a new instance starts with defaults. A frontend can distinguish
pending from applied values by tracking selections and successful transitions.
Unsupported reset support does not prevent choosing an option for the next load.

## Subsystem sessions

`count()` reports 0 through 16 descriptions. A `corvus_subsystem_info` has a
stable ID and label, 1 through 4 players, and 1 through 4 ordered content slots.
Its `games` array identifies each slot's label, accepted extension list and
`required` boolean (0 or 1). Player and slot indexes start at zero. Metadata is a
public description of what a core can compose, not permission to access private
engine state.

`load(core, id, games, count)` operates only on a live unloaded base instance.
The ID must match a description and `count` must exactly equal `game_count`.
Every entry initializes `struct_size`; an absent optional slot has NULL buffers
and zero sizes. Required slots need supported nonempty content. ROM and battery
bytes belong to the host during the call. The core copies every byte it retains
before returning, so the host may release both buffers immediately afterwards.

The load is transactional. Invalid IDs, counts, structures, content or
allocation failures return 0 with the instance unloaded and reusable, without
video/audio callbacks or a partial player session. Calling it on an already
loaded instance returns 0 and preserves the existing session. Base `load_game`
likewise must not replace a loaded subsystem session. Unload returns the same
instance to the unloaded state; no second kind of core handle is introduced.

After success, base `get_av_info`, `set_button`, `run_frame`, `reset` and
`unload_game` operate the complete session. The core composes the complete video
frame and one audio stream; the frontend does not run separate private engines.
The metadata declares valid player indexes. There is no universal layout or
mixing policy: the synthetic example uses adjacent pixels and a sum of tones.

## Current-session capabilities and states

`capabilities(core)` returns 0 for NULL or unloaded handles. When the extension
exists, query it after **both** ordinary base `load_game` and subsystem loads.
Version 1 defines `CORVUS_SESSION_RESET == 1` and `CORVUS_SESSION_STATES == 2`.
These bits describe support for the entire currently loaded session; they are
not static module capabilities.

A frontend enables reset only when RESET is present **and** the base `reset`
pointer is non-NULL. It enables states only when STATES is present **and** all
three base state pointers are non-NULL. A complete base group alone is not proof
of session support. Without STATES the frontend explicitly disables save/load
state, and the core returns zero/failure for base state operations on that
session. Never serialize only player 1 of a multiple-content session. With
STATES, serialization covers the complete session or fails atomically. With no
subsystem extension at all, use the historical base pointer rules.

## Independent battery generations

The optional per-slot battery group operates on ordered content slots, including
slot 0 of a normal base `load_game` session. Invalid or unloaded slots have size
and dirty equal to 0, and copy/acknowledge fail. Failed copies do not modify the
host buffer or generation output. A successful copy requires enough capacity
and a non-NULL generation pointer and returns the snapshot's generation.

Generations and dirty tracking are independent per slot. Acknowledge a copied
generation only after the frontend has durably persisted that slot's snapshot.
Acknowledging an older copied generation must leave newer writes dirty. Never
use one player's generation to acknowledge another player's save; the frontend
also discards pending asynchronous acknowledgements when a session is unloaded.
Failure to persist leaves the slot dirty for a later retry.

For sessions with multiple contents, the base battery group reports unavailable:
`battery_size` and `battery_dirty` return 0, and `battery_copy` and
`battery_acknowledge` return 0. Its function pointers may remain present for
normal sessions. This prevents accidentally saving just player 1. Frontends use
the complete per-slot group for multiple contents, or report battery persistence
unavailable when the group is absent. They must never reinterpret private save
formats to compensate.

## Synthetic implementation and verification

`examples/minimal_core.c` remains a legacy module with neither optional symbol.
`examples/configurable_core.c` adds immediate `invert`, restart-required `bias`,
a two-slot `pair` subsystem, per-player input, composed video/audio and two
independent one-byte saves. It supports reset but explicitly does not support
states, in either ordinary or paired sessions. The example retains only the
first byte of each synthetic ROM and battery buffer.

`tests/configuration_test.c` verifies negotiation failures without writes,
metadata, option timing and invalid selections, copied input ownership,
transactional paired load failure, loaded-session preservation, capability
changes, independent dirty generations and base battery unavailability.
The verifier accepts both examples and validates any advertised optional groups
and bounded metadata before dispatch. These checks demonstrate the public
contract; they do not prove emulator fidelity or safety of untrusted modules.
