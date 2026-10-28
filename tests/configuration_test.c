// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

#include <corvusapi.h>
#include <stdio.h>
#include <string.h>

#define REQUIRE(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
    return 1; \
} } while (0)
#define NEGOTIATION(entry, type, version) do { \
    struct { type api; uint64_t guard; } result; \
    type before; \
    memset(&result, 0xA5, sizeof(result)); \
    result.api.struct_size = sizeof(result.api); memcpy(&before, &result.api, sizeof(before)); \
    REQUIRE(!(entry)((version) + 1u, &result.api)); \
    REQUIRE(!memcmp(&before, &result.api, sizeof(before))); \
    result.api.struct_size = sizeof(result.api) - 1u; memcpy(&before, &result.api, sizeof(before)); \
    REQUIRE(!(entry)((version), &result.api)); \
    REQUIRE(!memcmp(&before, &result.api, sizeof(before))); \
    REQUIRE(!(entry)((version), NULL)); \
    result.api.struct_size = sizeof(result); \
    uint64_t guard = result.guard; \
    REQUIRE((entry)((version), &result.api)); \
    REQUIRE(result.guard == guard); \
    REQUIRE(result.api.struct_size == sizeof(type)); \
} while (0)

typedef struct observed {
    unsigned video, audio;
    uint32_t width, pixel[2];
    int16_t sample;
} observed;
static void video(void *userdata, const uint32_t *pixels, uint32_t width,
                  uint32_t height, size_t stride) {
    observed *o = (observed *)userdata;
    ++o->video;
    o->width = height == 1 && stride == width ? width : 0;
    o->pixel[0] = pixels[0];
    o->pixel[1] = width == 2 ? pixels[1] : 0;
}
static void audio(void *userdata, const int16_t *samples, size_t frames) {
    observed *o = (observed *)userdata;
    ++o->audio;
    o->sample = frames == 800 ? samples[0] : -1;
}
int main(void) {
    NEGOTIATION(corvus_get_options_api, corvus_options_api, CORVUS_OPTIONS_API_VERSION);
    NEGOTIATION(corvus_get_subsystem_api, corvus_subsystem_api, CORVUS_SUBSYSTEM_API_VERSION);
    corvus_core_api base = {.struct_size = sizeof(base)};
    corvus_options_api options = {.struct_size = sizeof(options)};
    corvus_subsystem_api subs = {.struct_size = sizeof(subs)};
    REQUIRE(corvus_get_api(CORVUS_API_VERSION, &base));
    REQUIRE(corvus_get_options_api(CORVUS_OPTIONS_API_VERSION, &options));
    REQUIRE(corvus_get_subsystem_api(CORVUS_SUBSYSTEM_API_VERSION, &subs));
    REQUIRE(options.api_version == 1 && subs.api_version == 1);
    REQUIRE(options.count && options.describe && options.get_value && options.set_value);
    REQUIRE(subs.count && subs.describe && subs.load && subs.capabilities);
    REQUIRE(subs.battery_size && subs.battery_dirty && subs.battery_copy && subs.battery_acknowledge);
    REQUIRE(!options.count(NULL) && options.get_value(NULL, "invert") == UINT32_MAX);
    REQUIRE(!options.set_value(NULL, "invert", 0));
    REQUIRE(!subs.capabilities(NULL) && !subs.battery_size(NULL, 0));
    REQUIRE(!subs.battery_dirty(NULL, 0) && !subs.battery_acknowledge(NULL, 0, 1));
    REQUIRE(!subs.load(NULL, "pair", NULL, 2));

    observed out = {0};
    corvus_host host = {.struct_size = sizeof(host), .userdata = &out,
        .video_frame = video, .audio_frames = audio};
    void *core = base.create(&host);
    REQUIRE(core && options.count(core) == 2 && subs.count() == 1);
    REQUIRE(!subs.capabilities(core) && !base.reset(core));
    corvus_core_option option = {.struct_size = sizeof(option)};
    REQUIRE(options.describe(core, 0, &option));
    REQUIRE(!strcmp(option.key, "invert") && option.value_count == 2 && !option.flags);
    REQUIRE(option.default_index == 0 && option.values[1].struct_size == sizeof(corvus_option_value));
    const char *stable_key = option.key;
    REQUIRE(!strcmp(option.values[1].id, "on"));
    corvus_core_option before;
    memcpy(&before, &option, sizeof(before));
    REQUIRE(!options.describe(core, 2, &option));
    REQUIRE(!memcmp(&before, &option, sizeof(option)));
    REQUIRE(!options.describe(NULL, 0, &option) && !options.describe(core, 0, NULL));
    option.struct_size = sizeof(option) - 1;
    memcpy(&before, &option, sizeof(before));
    REQUIRE(!options.describe(core, 0, &option));
    REQUIRE(!memcmp(&before, &option, sizeof(option)));
    option.struct_size = sizeof(option);
    REQUIRE(options.describe(core, 1, &option));
    REQUIRE(option.flags == CORVUS_OPTION_RESTART_REQUIRED);
    REQUIRE(options.get_value(core, NULL) == UINT32_MAX);
    REQUIRE(options.get_value(core, "unknown") == UINT32_MAX);
    REQUIRE(!options.set_value(core, NULL, 0) && !options.set_value(core, "unknown", 1));
    REQUIRE(!options.set_value(core, "invert", 2) && options.get_value(core, "invert") == 0);

    corvus_subsystem_info info = {.struct_size = sizeof(info)};
    REQUIRE(subs.describe(0, &info));
    REQUIRE(!strcmp(info.id, "pair") && info.game_count == 2 && info.num_players == 2);
    REQUIRE(info.games[0].required && info.games[1].required);
    REQUIRE(info.games[1].struct_size == sizeof(corvus_subsystem_rom));
    corvus_subsystem_info info_before;
    memcpy(&info_before, &info, sizeof(info_before));
    REQUIRE(!subs.describe(1, &info) && !subs.describe(0, NULL));
    REQUIRE(!memcmp(&info_before, &info, sizeof(info)));
    info.struct_size = sizeof(info) - 1;
    memcpy(&info_before, &info, sizeof(info_before));
    REQUIRE(!subs.describe(0, &info));
    REQUIRE(!memcmp(&info_before, &info, sizeof(info)));

    uint8_t rom[2] = {20, 40}, battery[2] = {7, 9};
    corvus_game games[2] = {
        {sizeof(corvus_game), &rom[0], 1, &battery[0], 1},
        {sizeof(corvus_game), &rom[1], 1, &battery[1], 1}
    };
    REQUIRE(base.load_game(core, &games[0]));
    REQUIRE(subs.capabilities(core) == CORVUS_SESSION_RESET);
    REQUIRE(base.reset && !base.serialize && !base.serialize_size && !base.unserialize);
    REQUIRE(base.run_frame(core) && out.pixel[0] == UINT32_C(0xFF141414));
    REQUIRE(options.set_value(core, "invert", 1));
    REQUIRE(out.video == 1 && out.audio == 1);
    REQUIRE(base.run_frame(core) && out.pixel[0] == UINT32_C(0xFFEBEBEB));
    REQUIRE(options.set_value(core, "invert", 0) && options.set_value(core, "bias", 1));
    REQUIRE(options.get_value(core, "bias") == 1);
    REQUIRE(base.run_frame(core) && out.pixel[0] == UINT32_C(0xFF141414));
    REQUIRE(!options.set_value(core, "bias", UINT32_MAX));
    REQUIRE(options.get_value(core, "bias") == 1);
    REQUIRE(base.reset(core));
    REQUIRE(base.run_frame(core) && out.pixel[0] == UINT32_C(0xFF151515));
    REQUIRE(base.battery_size(core) == 1 && subs.battery_size(core, 0) == 1);
    REQUIRE(!subs.load(core, "pair", games, 2));
    REQUIRE(base.run_frame(core) && out.width == 1 && out.pixel[0] == UINT32_C(0xFF151515));
    base.unload_game(core);
    REQUIRE(!subs.capabilities(core) && !subs.battery_size(core, 0));
    REQUIRE(options.get_value(core, "bias") == 1 && !strcmp(stable_key, "invert"));

    unsigned videos = out.video, audios = out.audio;
    REQUIRE(!subs.load(core, NULL, games, 2) && !subs.load(core, "unknown", games, 2));
    REQUIRE(!subs.load(core, "pair", games, 1) && !subs.load(core, "pair", NULL, 2));
    games[1].rom_data = NULL;
    REQUIRE(!subs.load(core, "pair", games, 2));
    REQUIRE(!subs.capabilities(core) && !base.run_frame(core));
    games[1].rom_data = &rom[1];
    REQUIRE(subs.load(core, "pair", games, 2));
    REQUIRE(out.video == videos && out.audio == audios);
    rom[0] = rom[1] = battery[0] = battery[1] = 99;
    REQUIRE(base.run_frame(core) && out.width == 2);
    REQUIRE(out.pixel[0] == UINT32_C(0xFF151515) && out.pixel[1] == UINT32_C(0xFF292929));
    REQUIRE(out.sample == 62 && subs.capabilities(core) == CORVUS_SESSION_RESET);
    REQUIRE(!(subs.capabilities(core) & CORVUS_SESSION_STATES));
    corvus_av_info av = {.struct_size = sizeof(av)};
    REQUIRE(base.get_av_info(core, &av) && av.width == 2 && av.height == 1);
    REQUIRE(!base.load_game(core, &games[0]) && !subs.load(core, "pair", games, 2));

    uint8_t copy = 0xEE;
    uint64_t generation = UINT64_MAX, generation0, generation1;
    REQUIRE(!base.battery_size(core) && !base.battery_dirty(core));
    REQUIRE(!base.battery_copy(core, &copy, 1, &generation));
    REQUIRE(!base.battery_acknowledge(core, 1) && copy == 0xEE && generation == UINT64_MAX);
    REQUIRE(!subs.battery_size(core, 2) && !subs.battery_dirty(core, 2));
    REQUIRE(!subs.battery_copy(core, 2, &copy, 1, &generation));
    REQUIRE(!subs.battery_copy(core, 0, NULL, 1, &generation));
    REQUIRE(!subs.battery_copy(core, 0, &copy, 0, &generation));
    REQUIRE(!subs.battery_copy(core, 0, &copy, 1, NULL));
    REQUIRE(copy == 0xEE && generation == UINT64_MAX);
    REQUIRE(subs.battery_copy(core, 0, &copy, 1, &generation0) && copy == 7);
    REQUIRE(subs.battery_copy(core, 1, &copy, 1, &generation1) && copy == 9);
    REQUIRE(!subs.battery_dirty(core, 0) && !subs.battery_dirty(core, 1));
    base.set_button(core, 1, CORVUS_BUTTON_A, 1);
    REQUIRE(base.run_frame(core));
    REQUIRE(out.pixel[0] == UINT32_C(0xFF151515) && out.pixel[1] == UINT32_C(0xFFD6D6D6));
    REQUIRE(!subs.battery_dirty(core, 0) && subs.battery_dirty(core, 1));
    REQUIRE(subs.battery_acknowledge(core, 1, generation1));
    REQUIRE(subs.battery_dirty(core, 1)); /* Old snapshot cannot clear newer writes. */
    REQUIRE(subs.battery_copy(core, 1, &copy, 1, &generation1) && copy == 10);
    REQUIRE(!subs.battery_acknowledge(core, 1, generation1 + 1));
    REQUIRE(subs.battery_acknowledge(core, 1, generation1) && !subs.battery_dirty(core, 1));
    REQUIRE(!subs.battery_dirty(core, 0));
    REQUIRE(base.reset(core) && base.run_frame(core));
    REQUIRE(out.pixel[1] == UINT32_C(0xFF292929));
    REQUIRE(options.set_value(core, "bias", 0));
    REQUIRE(base.run_frame(core) && out.pixel[0] == UINT32_C(0xFF151515));
    base.unload_game(core);
    REQUIRE(!subs.capabilities(core) && !subs.battery_dirty(core, 1));
    REQUIRE(!subs.battery_copy(core, 0, &copy, 1, &generation));
    REQUIRE(!subs.battery_acknowledge(core, 1, generation1));
    REQUIRE(base.load_game(core, &games[0]));
    REQUIRE(base.run_frame(core) && out.pixel[0] == UINT32_C(0xFF636363));
    REQUIRE(!subs.battery_acknowledge(core, 0, generation0));
    base.destroy(core);
    REQUIRE(!strcmp(stable_key, "invert")); /* Metadata has module lifetime. */
    puts("PASS: optional configuration and paired synthetic session");
    return 0;
}
