// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

/* Standalone ABI verifier for compiled CorvusAPI core modules.
 * Not an emulator and not a sample core. */
#include <corvusapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
typedef HMODULE module_handle;
static module_handle open_module(const char *path) { return LoadLibraryA(path); }
static void *lookup_symbol(module_handle module) {
    return (void *)GetProcAddress(module, CORVUS_CORE_ENTRYPOINT);
}
static void close_module(module_handle module) { FreeLibrary(module); }
#else
#include <dlfcn.h>
typedef void *module_handle;
static module_handle open_module(const char *path) { return dlopen(path, RTLD_NOW | RTLD_LOCAL); }
static void *lookup_symbol(module_handle module) {
    return dlsym(module, CORVUS_CORE_ENTRYPOINT);
}
static void close_module(module_handle module) { dlclose(module); }
#endif

static int valid_text(const char *s) {
    if (!s) return 0;
    size_t count = 0;
    while (count < 128 && s[count]) ++count;
    return count > 0 && count < 128;
}

static int fail(module_handle module, const char *reason) {
    fprintf(stderr, "CorvusAPI ABI rejected: %s\n", reason);
    if (module) close_module(module);
    return 1;
}

static void video_frame(void *context, const uint32_t *pixels, uint32_t width,
                        uint32_t height, size_t stride) {
    (void)context; (void)pixels; (void)width; (void)height; (void)stride;
}
static void audio_frames(void *context, const int16_t *samples, size_t frames) {
    (void)context; (void)samples; (void)frames;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("Usage: corvusapi-verify <core-native-library>\n", stderr);
        return 2;
    }
    module_handle module = open_module(argv[1]);
    if (!module) return fail(module, "cannot open core module");
    void *symbol = lookup_symbol(module);
    if (!symbol) return fail(module, "missing corvus_get_api");
    corvus_get_api_fn entry = NULL;
    memcpy(&entry, &symbol, sizeof(entry));

    struct {
        corvus_core_api api;
        uint64_t guard;
    } handshake = {{0}, UINT64_C(0x6C6973745F636F72)};
    handshake.api.struct_size = sizeof(handshake.api);
    if (!entry(CORVUS_API_VERSION, &handshake.api))
        return fail(module, "core rejects supported ABI");
    if (handshake.guard != UINT64_C(0x6C6973745F636F72))
        return fail(module, "core writes outside the announced ABI structure");

    corvus_core_api *api = &handshake.api;
    if (api->api_version != CORVUS_API_VERSION ||
        api->struct_size < sizeof(corvus_core_api))
        return fail(module, "ABI version or structure size mismatch");
    if (!api->get_info || !api->create || !api->destroy || !api->load_game ||
        !api->unload_game || !api->get_av_info || !api->set_button || !api->run_frame)
        return fail(module, "missing required entrypoint");

    int any_state = api->serialize_size || api->serialize || api->unserialize;
    int full_state = api->serialize_size && api->serialize && api->unserialize;
    int any_battery = api->battery_size || api->battery_dirty ||
                      api->battery_copy || api->battery_acknowledge;
    int full_battery = api->battery_size && api->battery_dirty &&
                       api->battery_copy && api->battery_acknowledge;
    if (any_state != full_state || any_battery != full_battery)
        return fail(module, "partial optional capability group");

    corvus_core_info info = {.struct_size = sizeof(info)};
    if (!api->get_info(&info) || info.struct_size < sizeof(info) ||
        !valid_text(info.id) || !valid_text(info.name) ||
        !valid_text(info.system_name) || !valid_text(info.version) ||
        !valid_text(info.extensions) || !info.max_players)
        return fail(module, "invalid core metadata");

    corvus_host host = {.struct_size = sizeof(host), .video_frame = video_frame,
                        .audio_frames = audio_frames};
    void *instance = api->create(&host);
    if (!instance) return fail(module, "cannot create core without game");
    api->destroy(instance);
    printf("PASS CorvusAPI %u: %s (%s), system=%s, formats=%s, state=%s, battery=%s\n",
           api->api_version, info.name, info.id, info.system_name, info.extensions,
           full_state ? "yes" : "no", full_battery ? "yes" : "no");
    close_module(module);
    return 0;
}
