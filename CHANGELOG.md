# Changelog

Copyright (C) 2026 CorvusSoftware. SPDX-License-Identifier: MIT.

## Unreleased

- Add optional version-1 options and subsystem C interfaces in `corvusapi.h`.
  ABI 0 structures, `corvus_get_api`, the achievements extension and package
  version 0.1.0 remain unchanged; missing extension symbols remain supported.
- Specify bounded immutable metadata, immediate and restart-required options,
  transactional multi-content loads, session capability flags and independent
  per-slot battery generations. Multi-content base battery operations are
  unavailable, and unsupported session state operations are explicitly disabled.
- Add a standalone synthetic configurable/paired core and contract tests.
  Extend native verification to inspect optional groups and bounded metadata.
