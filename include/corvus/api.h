// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

#ifndef CORVUS_API_H
#define CORVUS_API_H

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

#endif /* CORVUS_API_H */
