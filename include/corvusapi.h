// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

/* Standalone C11/C++20 public core ABI. Version 0 is experimental.
 * Optional extensions use separate versions and exported symbols. */
#ifndef CORVUSAPI_H
#define CORVUSAPI_H

#include <stddef.h>
#include <stdint.h>

/* ABI experimentale. La valeur change lorsqu'une rupture est introduite. */
#define CORVUS_API_VERSION 0u
#define CORVUS_CORE_ENTRYPOINT "corvus_get_api"

#if defined(_WIN32)
#define CORVUS_API_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define CORVUS_API_EXPORT __attribute__((visibility("default")))
#else
#define CORVUS_API_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Les pixels sont des entiers ARGB8888 (0xAARRGGBB).
 * Le son est du PCM signe 16 bits stereo, entrelace L/R.
 * Tous les callbacks sont synchrones et les buffers ne survivent pas a l'appel. */
typedef struct corvus_host {
    uint32_t struct_size;
    void *userdata;
    void (*video_frame)(void *userdata, const uint32_t *pixels,
                        uint32_t width, uint32_t height, size_t stride_pixels);
    void (*audio_frames)(void *userdata, const int16_t *samples, size_t stereo_frames);
    void (*log)(void *userdata, int level, const char *message);
} corvus_host;

enum corvus_log_level {
    CORVUS_LOG_DEBUG = 0,
    CORVUS_LOG_INFO = 1,
    CORVUS_LOG_WARNING = 2,
    CORVUS_LOG_ERROR = 3
};

enum corvus_button {
    CORVUS_BUTTON_UP = 0,
    CORVUS_BUTTON_DOWN = 1,
    CORVUS_BUTTON_LEFT = 2,
    CORVUS_BUTTON_RIGHT = 3,
    CORVUS_BUTTON_A = 4,
    CORVUS_BUTTON_B = 5,
    CORVUS_BUTTON_X = 6,
    CORVUS_BUTTON_Y = 7,
    CORVUS_BUTTON_START = 8,
    CORVUS_BUTTON_SELECT = 9,
    CORVUS_BUTTON_L = 10,
    CORVUS_BUTTON_R = 11,
    CORVUS_BUTTON_L2 = 12,
    CORVUS_BUTTON_R2 = 13,
    CORVUS_BUTTON_L3 = 14,
    CORVUS_BUTTON_R3 = 15
};

/* Les chaines sont en UTF-8 et restent valides pendant la vie du module.
 * Les extensions sont separees par '|', sans point initial (ex. gb|gbc). */
typedef struct corvus_core_info {
    uint32_t struct_size;
    const char *id;
    const char *name;
    const char *version;
    const char *system_name;
    const char *extensions;
    uint32_t max_players;
} corvus_core_info;

/* Les donnees ROM et batterie appartiennent a l'hote.
 * Le coeur copie ce qu'il conserve avant de retourner de load_game. */
typedef struct corvus_game {
    uint32_t struct_size;
    const uint8_t *rom_data;
    size_t rom_size;
    const uint8_t *battery_data;
    size_t battery_size;
} corvus_game;

typedef struct corvus_av_info {
    uint32_t struct_size;
    uint32_t width;
    uint32_t height;
    double fps;
    uint32_t audio_sample_rate;
} corvus_av_info;

/* API partagee par tous les coeurs. Aucun type C++ ou Android n'est expose.
 * Le coeur est un handle opaque propre a une instance.
 *
 * Fonctions obligatoires :
 * get_info, create, destroy, load_game, unload_game, get_av_info,
 * set_button, run_frame.
 *
 * Fonctions optionnelles :
 * reset, last_error, groupe serialize_*, groupe battery_*.
 * Un groupe optionnel est present en entier ou absent en entier. */
typedef struct corvus_core_api {
    uint32_t struct_size;
    uint32_t api_version;

    int (*get_info)(corvus_core_info *out);
    void *(*create)(const corvus_host *host);
    void (*destroy)(void *core);
    int (*load_game)(void *core, const corvus_game *game);
    void (*unload_game)(void *core);
    int (*get_av_info)(void *core, corvus_av_info *out);
    void (*set_button)(void *core, uint32_t player, uint32_t button, int pressed);
    int (*run_frame)(void *core);
    int (*reset)(void *core);
    const char *(*last_error)(void *core);

    size_t (*serialize_size)(void *core);
    int (*serialize)(void *core, uint8_t *output, size_t capacity);
    int (*unserialize)(void *core, const uint8_t *data, size_t size);

    size_t (*battery_size)(void *core);
    int (*battery_dirty)(void *core);
    int (*battery_copy)(void *core, uint8_t *output, size_t capacity,
                        uint64_t *generation);
    int (*battery_acknowledge)(void *core, uint64_t generation);
} corvus_core_api;

/* Symbole C exporte par chaque coeur. L'hote initialise out a zero,
 * fixe out->struct_size a sizeof(corvus_core_api) et demande
 * CORVUS_API_VERSION. Retourne 1 en cas de compatibilite, sinon 0.
 * Le coeur ne doit jamais ecrire au-dela de out->struct_size.
 * Les pointeurs de fonctions restent valides tant que le module est charge. */
typedef int (*corvus_get_api_fn)(uint32_t requested_version, corvus_core_api *out);
CORVUS_API_EXPORT int corvus_get_api(uint32_t requested_version,
                                     corvus_core_api *out);

#ifdef __cplusplus
}
#endif

/* Optional achievement-memory extension. Modules without this
 * exported symbol still implement the base CorvusAPI ABI. */
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

/* Optional configuration extensions. These never change corvus_core_api. */
#define CORVUS_OPTIONS_API_VERSION 1u
#define CORVUS_OPTIONS_ENTRYPOINT "corvus_get_options_api"
#define CORVUS_OPTION_RESTART_REQUIRED (1u << 0)
#define CORVUS_SUBSYSTEM_API_VERSION 1u
#define CORVUS_SUBSYSTEM_ENTRYPOINT "corvus_get_subsystem_api"
#define CORVUS_SESSION_RESET (1u << 0)
#define CORVUS_SESSION_STATES (1u << 1)

#ifdef __cplusplus
extern "C" {
#endif

/* Immutable module-lifetime metadata. Keys and value IDs are nonempty ASCII
 * machine tokens [A-Za-z0-9_.-], at most 95 bytes. Labels and descriptions are
 * UTF-8, at most 255 bytes excluding NUL. At most 64 options/64 values each.
 * Every option has at least one value and a valid default_index. Keys are unique
 * per module; value IDs are unique within their option. Metadata, counts and
 * array shapes do not change during the module lifetime, including selections,
 * load and unload. Arrays use the native version-1 struct layout/stride. */
typedef struct corvus_option_value {
    uint32_t struct_size;
    const char *id;
    const char *label;
} corvus_option_value;

typedef struct corvus_core_option {
    uint32_t struct_size;
    const char *key;
    const char *label;
    const char *description;
    uint32_t flags;
    uint32_t value_count;
    const corvus_option_value *values;
    uint32_t default_index;
} corvus_core_option;

/* Same base create handle; calls serialized on its owner thread between frames.
 * Metadata is available before content load. count(NULL) returns 0;
 * describe rejects NULL/unrecognized index/small output
 * without writing. get_value returns the selected (possibly pending) index, or
 * UINT32_MAX for NULL/unknown key. set_value rejects invalid input atomically.
 * Defaults are selected on create. Unflagged selections apply immediately;
 * RESTART_REQUIRED applies on the next successful load or reset. No setter may
 * invoke video/audio callbacks. Unload preserves the selected options. */
typedef struct corvus_options_api {
    uint32_t struct_size;
    uint32_t api_version;
    uint32_t (*count)(void *core);
    int (*describe)(void *core, uint32_t index, corvus_core_option *out);
    uint32_t (*get_value)(void *core, const char *key);
    int (*set_value)(void *core, const char *key, uint32_t value_index);
} corvus_options_api;

typedef int (*corvus_get_options_api_fn)(uint32_t requested_version,
                                        corvus_options_api *out);
CORVUS_API_EXPORT int corvus_get_options_api(uint32_t requested_version,
                                            corvus_options_api *out);

/* Subsystem IDs use the same token rules as option keys. All strings and arrays
 * are immutable for the module lifetime. Extensions are pipe-separated suffixes
 * without dots. IDs are unique per module. Labels/extensions are nonempty UTF-8
 * strings, at most 255 bytes excluding NUL. Metadata/count are available before
 * load, fixed for module lifetime; arrays use native version-1 layout/stride.
 * There are at most 16 subsystems, each with 1..4 players and
 * 1..4 ordered content slots. required is 0 or 1. */
typedef struct corvus_subsystem_rom {
    uint32_t struct_size;
    const char *label;
    const char *extensions;
    uint32_t required;
} corvus_subsystem_rom;

typedef struct corvus_subsystem_info {
    uint32_t struct_size;
    const char *id;
    const char *label;
    uint32_t num_players;
    uint32_t game_count;
    const corvus_subsystem_rom *games;
} corvus_subsystem_info;

/* load accepts only a live UNLOADED base handle. count equals game_count;
 * absent optional slots are zeroed corvus_game with struct_size set. All ROM
 * and battery bytes retained are copied before returning. Failure leaves the
 * instance unloaded/reusable; loading an already loaded handle fails unchanged.
 * No callbacks are emitted by load. Base run_frame, inputs, AV, unload and reset
 * operate the complete session; the core composes video/audio for all players.
 * capabilities is 0 unloaded and also describes normal load_game sessions.
 * RESET/STATES must be intersected with base reset/full state pointer support.
 * Without STATES, never serialize an individual player of a paired session.
 * For multiple contents, base battery reports unavailable (size/dirty=0,
 * copy/acknowledge=0). The per-slot battery group is all present or all NULL,
 * uses zero-based slots and independent generations, including normal slot 0.
 * Invalid/unloaded slots return 0 without writing buffers or generation.
 * Acknowledgement follows durable persistence and cannot clear newer writes.
 * All instance operations follow the base owner-thread and callback rules. */
typedef struct corvus_subsystem_api {
    uint32_t struct_size;
    uint32_t api_version;
    uint32_t (*count)(void);
    int (*describe)(uint32_t index, corvus_subsystem_info *out);
    int (*load)(void *core, const char *id, const corvus_game *games, size_t count);
    uint32_t (*capabilities)(void *core);
    size_t (*battery_size)(void *core, uint32_t slot);
    int (*battery_dirty)(void *core, uint32_t slot);
    int (*battery_copy)(void *core, uint32_t slot, uint8_t *output,
                        size_t capacity, uint64_t *generation);
    int (*battery_acknowledge)(void *core, uint32_t slot, uint64_t generation);
} corvus_subsystem_api;

typedef int (*corvus_get_subsystem_api_fn)(uint32_t requested_version,
                                          corvus_subsystem_api *out);
CORVUS_API_EXPORT int corvus_get_subsystem_api(uint32_t requested_version,
                                              corvus_subsystem_api *out);

/* Both negotiations reject NULL, unsupported versions and structures smaller
 * than the current complete API without writing any output bytes. Success fills
 * the current structure only. Missing symbols are supported legacy modules. */
#ifdef __cplusplus
}
#endif

#endif /* CORVUSAPI_H */
