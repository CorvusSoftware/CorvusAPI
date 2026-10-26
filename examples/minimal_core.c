// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

/* Minimal, independent native module demonstrating the public C ABI.
 * This is a synthetic color-frame core, NOT a commercial console emulator. */
#include <corvusapi.h>
#include <stdlib.h>

typedef struct example_instance {
    corvus_host host;
    int loaded;
    uint8_t tone;
    int a_pressed;
} example_instance;

static int example_info(corvus_core_info *out) {
    if (!out || out->struct_size < sizeof(*out)) return 0;
    out->struct_size = sizeof(*out);
    out->id = "org.corvussoftware.example";
    out->name = "CorvusAPI Example";
    out->version = "0.1.0";
    out->system_name = "Synthetic test core";
    out->extensions = "bin";
    out->max_players = 1;
    return 1;
}

static void *example_create(const corvus_host *host) {
    if (!host || host->struct_size < sizeof(*host) || !host->video_frame)
        return NULL;
    example_instance *instance = (example_instance *)calloc(1, sizeof(*instance));
    if (!instance) return NULL;
    instance->host = *host;
    return instance;
}

static void example_destroy(void *core) { free(core); }

static int example_load(void *core, const corvus_game *game) {
    example_instance *instance = (example_instance *)core;
    if (!instance || !game || game->struct_size < sizeof(*game) ||
        !game->rom_data || !game->rom_size) return 0;
    /* Only one byte is read. Never retain the host-owned ROM pointer. */
    instance->tone = game->rom_data[0];
    instance->loaded = 1;
    instance->a_pressed = 0;
    return 1;
}

static void example_unload(void *core) {
    example_instance *instance = (example_instance *)core;
    if (instance) instance->loaded = 0;
}

static int example_av(void *core, corvus_av_info *out) {
    example_instance *instance = (example_instance *)core;
    if (!instance || !instance->loaded || !out || out->struct_size < sizeof(*out))
        return 0;
    out->struct_size = sizeof(*out);
    out->width = 2;
    out->height = 2;
    out->fps = 60.0;
    out->audio_sample_rate = 48000;
    return 1;
}

static void example_button(void *core, uint32_t player, uint32_t button, int pressed) {
    example_instance *instance = (example_instance *)core;
    if (!instance || !instance->loaded || player != 0) return;
    if (button == CORVUS_BUTTON_A) instance->a_pressed = pressed != 0;
}

static int example_frame(void *core) {
    example_instance *instance = (example_instance *)core;
    if (!instance || !instance->loaded) return 0;
    uint8_t value = instance->a_pressed ? (uint8_t)(255u - instance->tone)
                                       : instance->tone;
    const uint32_t color = UINT32_C(0xFF000000) |
        ((uint32_t)value << 16) | ((uint32_t)value << 8) | value;
    const uint32_t pixels[4] = {color, color, color, color};
    instance->host.video_frame(instance->host.userdata, pixels, 2, 2, 2);
    if (instance->host.audio_frames) {
        /* 48000 / 60 = 800 stereo frames; silent PCM, L/R interleaved. */
        const int16_t samples[1600] = {0};
        instance->host.audio_frames(instance->host.userdata, samples, 800);
    }
    return 1;
}

static int example_reset(void *core) {
    example_instance *instance = (example_instance *)core;
    if (!instance || !instance->loaded) return 0;
    instance->a_pressed = 0;
    return 1;
}

CORVUS_API_EXPORT int corvus_get_api(uint32_t requested_version, corvus_core_api *out) {
    if (requested_version != CORVUS_API_VERSION ||
        !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_core_api api = {
        .struct_size = sizeof(corvus_core_api),
        .api_version = CORVUS_API_VERSION,
        .get_info = example_info,
        .create = example_create,
        .destroy = example_destroy,
        .load_game = example_load,
        .unload_game = example_unload,
        .get_av_info = example_av,
        .set_button = example_button,
        .run_frame = example_frame,
        .reset = example_reset
    };
    *out = api;
    return 1;
}
