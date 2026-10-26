// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT
#ifndef CORVUS_ACHIEVEMENTS_H
#define CORVUS_ACHIEVEMENTS_H

#include <corvus/api.h>

#define CORVUS_ACHIEVEMENTS_API_VERSION 1u
#define CORVUS_ACHIEVEMENTS_ENTRYPOINT "corvus_get_achievements_api"

#ifdef __cplusplus
extern "C" {
#endif

/* These identifiers and addresses are the public RetroAchievements console
 * address spaces, not physical CPU addresses on every machine. */
enum corvus_achievement_console {
    CORVUS_ACHIEVEMENT_GAME_BOY = 4,
    CORVUS_ACHIEVEMENT_GAME_BOY_ADVANCE = 5,
    CORVUS_ACHIEVEMENT_GAME_BOY_COLOR = 6
};

/* Optional extension. It does not change corvus_core_api or ABI version 0.
 * A missing symbol means that achievements are unsupported by this module.
 * The host initializes struct_size, zeroes other fields and negotiates version1.
 * console_id identifies the address space; other systems use their public IDs.
 *
 * read_memory receives the SAME opaque handle returned by corvus_core_api.create.
 * Call serially on the instance's owning thread, after a successful load_game and
 * between run_frame calls. No allocation, clock advance, bus side effects, state
 * mutation or battery acknowledgement is allowed. The host owns output.
 *
 * Return the contiguous prefix actually copied, at most capacity. Stop at the
 * first unmapped byte, including 32-bit address overflow. Never manufacture zero
 * bytes to make an unavailable address appear supported. A NULL/unloaded handle,
 * NULL output or zero capacity returns zero. Other handle pointers must be live
 * instances created by this module. Function pointers remain
 * valid only while the corresponding core module remains loaded. */
typedef struct corvus_achievements_api {
    uint32_t struct_size;
    uint32_t api_version;
    uint32_t console_id;
    size_t (*read_memory)(void *core, uint32_t address, uint8_t *output,
                          size_t capacity);
} corvus_achievements_api;

typedef int (*corvus_get_achievements_api_fn)(uint32_t requested_version,
                                            corvus_achievements_api *out);
CORVUS_API_EXPORT int corvus_get_achievements_api(uint32_t requested_version,
                                                corvus_achievements_api *out);

#ifdef __cplusplus
}
#endif
#endif
