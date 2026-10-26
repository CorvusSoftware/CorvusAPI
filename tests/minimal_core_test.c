// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

/* Call into the example core without a frontend or a platform loader. */
#include <corvusapi.h>
#include <stdio.h>
#include <string.h>

typedef struct observed {
    unsigned frames;
    unsigned audio_calls;
    size_t audio_frames;
    uint32_t pixel;
    int invalid;
} observed;

static void video(void *ctx, const uint32_t *pixels, uint32_t width,
                  uint32_t height, size_t stride) {
    observed *out = (observed *)ctx;
    if (!pixels || width != 2 || height != 2 || stride != 2) {
        out->invalid = 1;
        return;
    }
    ++out->frames;
    out->pixel = pixels[0];
}

static void audio(void *ctx, const int16_t *samples, size_t frames) {
    observed *out = (observed *)ctx;
    if (!samples || frames != 800 || samples[0] != 0 || samples[1] != 0)
        out->invalid = 1;
    ++out->audio_calls;
    out->audio_frames += frames;
}

#define REQUIRE(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
    return 1; \
} } while (0)

int main(void) {
    corvus_core_api api = {0};
    api.struct_size = sizeof(api);
    REQUIRE(!corvus_get_api(CORVUS_API_VERSION + 1u, &api));
    REQUIRE(corvus_get_api(CORVUS_API_VERSION, &api));
    REQUIRE(api.api_version == CORVUS_API_VERSION);
    REQUIRE(api.struct_size == sizeof(api));
    REQUIRE(api.get_info && api.create && api.destroy && api.load_game);
    REQUIRE(api.unload_game && api.get_av_info && api.set_button && api.run_frame);

    corvus_core_info info = {0};
    info.struct_size = sizeof(info);
    REQUIRE(api.get_info(&info));
    REQUIRE(info.id && info.name && info.extensions && info.max_players == 1);

    observed counts = {0};
    corvus_host host = {0};
    host.struct_size = sizeof(host);
    host.userdata = &counts;
    host.video_frame = video;
    host.audio_frames = audio;
    void *instance = api.create(&host);
    REQUIRE(instance != NULL);
    REQUIRE(!api.run_frame(instance));

    uint8_t rom[] = {42};
    corvus_game game = {0};
    game.struct_size = sizeof(game);
    game.rom_data = rom;
    game.rom_size = sizeof(rom);
    REQUIRE(api.load_game(instance, &game));
    /* The example copied the only byte it needs. */
    rom[0] = 99;

    corvus_av_info av = {0};
    av.struct_size = sizeof(av);
    REQUIRE(api.get_av_info(instance, &av));
    REQUIRE(av.width == 2 && av.height == 2 && av.fps == 60.0);
    REQUIRE(av.audio_sample_rate == 48000);

    REQUIRE(api.run_frame(instance));
    REQUIRE(counts.pixel == UINT32_C(0xFF2A2A2A));
    api.set_button(instance, 0, CORVUS_BUTTON_A, 1);
    REQUIRE(api.run_frame(instance));
    REQUIRE(counts.pixel == UINT32_C(0xFFD5D5D5));
    REQUIRE(api.reset && api.reset(instance));
    REQUIRE(api.run_frame(instance));
    REQUIRE(counts.pixel == UINT32_C(0xFF2A2A2A));
    REQUIRE(counts.frames == 3 && counts.audio_calls == 3);
    REQUIRE(counts.audio_frames == 2400 && !counts.invalid);

    REQUIRE(!api.serialize && !api.serialize_size && !api.unserialize);
    REQUIRE(!api.battery_copy && !api.battery_size);
    api.unload_game(instance);
    REQUIRE(!api.run_frame(instance));
    api.destroy(instance);
    puts("PASS: CorvusAPI example lifecycle");
    return 0;
}
