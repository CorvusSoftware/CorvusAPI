// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

/* Synthetic options and paired-content session, with no emulator dependency. */
#include <corvusapi.h>
#include <stdlib.h>
#include <string.h>

typedef struct configurable_instance {
    corvus_host host;
    uint32_t slots;
    uint32_t selected[2];
    uint32_t applied_bias;
    uint8_t tone[2], battery[2];
    int pressed[2];
    uint64_t generation[2], acknowledged[2], copied[2];
} configurable_instance;

static const corvus_option_value switches[] = {
    {sizeof(corvus_option_value), "off", "Off"},
    {sizeof(corvus_option_value), "on", "On"}
};
static const corvus_core_option options[] = {
    {sizeof(corvus_core_option), "invert", "Invert", "Invert both video tones immediately.",
     0, 2, switches, 0},
    {sizeof(corvus_core_option), "bias", "Tone bias", "Add one on the next load or reset.",
     CORVUS_OPTION_RESTART_REQUIRED, 2, switches, 0}
};
static const corvus_subsystem_rom slots[] = {
    {sizeof(corvus_subsystem_rom), "Player 1", "bin", 1},
    {sizeof(corvus_subsystem_rom), "Player 2", "bin", 1}
};
static const corvus_subsystem_info pair = {
    sizeof(corvus_subsystem_info), "pair", "Synthetic pair", 2, 2, slots
};

static int info(corvus_core_info *out) {
    if (!out || out->struct_size < sizeof(*out)) return 0;
    const corvus_core_info value = {sizeof(value), "org.corvussoftware.configurable",
        "Configurable synthetic core", "0.1.0", "Synthetic test core", "bin", 2};
    *out = value;
    return 1;
}
static void *create(const corvus_host *host) {
    if (!host || host->struct_size < sizeof(*host) || !host->video_frame) return NULL;
    configurable_instance *c = (configurable_instance *)calloc(1, sizeof(*c));
    if (c) c->host = *host;
    return c;
}
static void destroy(void *core) { free(core); }
static int valid_game(const corvus_game *game) {
    return game && game->struct_size >= sizeof(*game) && game->rom_data &&
        game->rom_size && (!game->battery_size || game->battery_data);
}
static int load_contents(configurable_instance *c, const corvus_game *games, size_t count) {
    if (!c || c->slots || !games || !count || count > 2) return 0;
    /* Validate the complete transaction before committing any session state. */
    for (size_t i = 0; i < count; ++i) if (!valid_game(&games[i])) return 0;
    for (size_t i = 0; i < count; ++i) {
        /* The synthetic engine only retains the first byte of each buffer. */
        c->tone[i] = games[i].rom_data[0];
        c->battery[i] = games[i].battery_size ? games[i].battery_data[0] : 0;
        c->pressed[i] = 0;
        /* Keep generations monotonic across sessions to reject stale acks. */
        ++c->generation[i];
        c->acknowledged[i] = c->generation[i];
        c->copied[i] = 0;
    }
    c->applied_bias = c->selected[1];
    c->slots = (uint32_t)count;
    return 1;
}
static int load_game(void *core, const corvus_game *game) {
    return load_contents((configurable_instance *)core, game, 1);
}
static void unload(void *core) {
    configurable_instance *c = (configurable_instance *)core;
    if (c) c->slots = 0;
}
static int av(void *core, corvus_av_info *out) {
    configurable_instance *c = (configurable_instance *)core;
    if (!c || !c->slots || !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_av_info value = {sizeof(value), c->slots, 1, 60.0, 48000};
    *out = value;
    return 1;
}
static void button(void *core, uint32_t player, uint32_t key, int pressed) {
    configurable_instance *c = (configurable_instance *)core;
    if (c && player < c->slots && key == CORVUS_BUTTON_A) c->pressed[player] = pressed != 0;
}
static int frame(void *core) {
    configurable_instance *c = (configurable_instance *)core;
    if (!c || !c->slots) return 0;
    uint32_t pixels[2];
    int16_t samples[1600] = {0};
    for (uint32_t i = 0; i < c->slots; ++i) {
        uint8_t tone = (uint8_t)(c->tone[i] + c->applied_bias);
        if (c->pressed[i] || c->selected[0]) tone = (uint8_t)(255u - tone);
        pixels[i] = UINT32_C(0xFF000000) | ((uint32_t)tone * UINT32_C(0x010101));
        if (c->pressed[i]) { ++c->battery[i]; ++c->generation[i]; }
        /* Mix both synthetic players in the one stereo stream. */
        samples[0] = samples[1] = (int16_t)(samples[0] + tone);
    }
    c->host.video_frame(c->host.userdata, pixels, c->slots, 1, c->slots);
    if (c->host.audio_frames) c->host.audio_frames(c->host.userdata, samples, 800);
    return 1;
}
static int reset(void *core) {
    configurable_instance *c = (configurable_instance *)core;
    if (!c || !c->slots) return 0;
    c->pressed[0] = c->pressed[1] = 0;
    c->applied_bias = c->selected[1];
    return 1;
}
static uint32_t option_count(void *core) { return core ? 2 : 0; }
static int option_describe(void *core, uint32_t index, corvus_core_option *out) {
    if (!core || index >= 2 || !out || out->struct_size < sizeof(*out)) return 0;
    *out = options[index];
    return 1;
}
static uint32_t option_index(const char *key) {
    if (key) for (uint32_t i = 0; i < 2; ++i) if (!strcmp(key, options[i].key)) return i;
    return UINT32_MAX;
}
static uint32_t option_get(void *core, const char *key) {
    configurable_instance *c = (configurable_instance *)core;
    uint32_t index = option_index(key);
    return c && index < 2 ? c->selected[index] : UINT32_MAX;
}
static int option_set(void *core, const char *key, uint32_t value) {
    configurable_instance *c = (configurable_instance *)core;
    uint32_t index = option_index(key);
    if (!c || index >= 2 || value >= options[index].value_count) return 0;
    c->selected[index] = value;
    return 1;
}
static uint32_t subsystem_count(void) { return 1; }
static int subsystem_describe(uint32_t index, corvus_subsystem_info *out) {
    if (index || !out || out->struct_size < sizeof(*out)) return 0;
    *out = pair;
    return 1;
}
static int subsystem_load(void *core, const char *id, const corvus_game *games, size_t count) {
    if (!id || strcmp(id, pair.id) || count != 2) return 0;
    return load_contents((configurable_instance *)core, games, count);
}
static uint32_t capabilities(void *core) {
    configurable_instance *c = (configurable_instance *)core;
    return c && c->slots ? CORVUS_SESSION_RESET : 0;
}
static size_t slot_size(void *core, uint32_t slot) {
    configurable_instance *c = (configurable_instance *)core;
    return c && slot < c->slots ? 1 : 0;
}
static int slot_dirty(void *core, uint32_t slot) {
    configurable_instance *c = (configurable_instance *)core;
    return slot_size(core, slot) && c->generation[slot] != c->acknowledged[slot];
}
static int slot_copy(void *core, uint32_t slot, uint8_t *out, size_t capacity, uint64_t *generation) {
    configurable_instance *c = (configurable_instance *)core;
    if (!slot_size(core, slot) || !out || capacity < 1 || !generation) return 0;
    out[0] = c->battery[slot];
    *generation = c->generation[slot];
    c->copied[slot] = *generation;
    return 1;
}
static int slot_acknowledge(void *core, uint32_t slot, uint64_t generation) {
    configurable_instance *c = (configurable_instance *)core;
    if (!slot_size(core, slot) || !generation || generation != c->copied[slot] ||
        generation < c->acknowledged[slot]) return 0;
    c->acknowledged[slot] = generation;
    return 1;
}
static size_t battery_size(void *core) {
    configurable_instance *c = (configurable_instance *)core;
    return c && c->slots == 1 ? slot_size(core, 0) : 0;
}
static int battery_dirty(void *core) { return battery_size(core) ? slot_dirty(core, 0) : 0; }
static int battery_copy(void *core, uint8_t *out, size_t capacity, uint64_t *generation) {
    return battery_size(core) ? slot_copy(core, 0, out, capacity, generation) : 0;
}
static int battery_acknowledge(void *core, uint64_t generation) {
    return battery_size(core) ? slot_acknowledge(core, 0, generation) : 0;
}
CORVUS_API_EXPORT int corvus_get_api(uint32_t version, corvus_core_api *out) {
    if (version != CORVUS_API_VERSION || !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_core_api api = {
        .struct_size = sizeof(api), .api_version = CORVUS_API_VERSION,
        .get_info = info, .create = create, .destroy = destroy, .load_game = load_game,
        .unload_game = unload, .get_av_info = av, .set_button = button,
        .run_frame = frame, .reset = reset,
        .battery_size = battery_size, .battery_dirty = battery_dirty,
        .battery_copy = battery_copy, .battery_acknowledge = battery_acknowledge
    };
    *out = api;
    return 1;
}
CORVUS_API_EXPORT int corvus_get_options_api(uint32_t version, corvus_options_api *out) {
    if (version != CORVUS_OPTIONS_API_VERSION || !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_options_api api = {sizeof(api), CORVUS_OPTIONS_API_VERSION,
        option_count, option_describe, option_get, option_set};
    *out = api;
    return 1;
}
CORVUS_API_EXPORT int corvus_get_subsystem_api(uint32_t version, corvus_subsystem_api *out) {
    if (version != CORVUS_SUBSYSTEM_API_VERSION || !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_subsystem_api api = {sizeof(api), CORVUS_SUBSYSTEM_API_VERSION,
        subsystem_count, subsystem_describe, subsystem_load, capabilities,
        slot_size, slot_dirty, slot_copy, slot_acknowledge};
    *out = api;
    return 1;
}
