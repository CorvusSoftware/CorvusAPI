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
static void *lookup_symbol(module_handle module, const char *name) {
    return (void *)GetProcAddress(module, name);
}
static void close_module(module_handle module) { FreeLibrary(module); }
#else
#include <dlfcn.h>
typedef void *module_handle;
static module_handle open_module(const char *path) { return dlopen(path, RTLD_NOW | RTLD_LOCAL); }
static void *lookup_symbol(module_handle module, const char *name) {
    return dlsym(module, name);
}
static void close_module(module_handle module) { dlclose(module); }
#endif

static int valid_text(const char *s) {
    if (!s) return 0;
    size_t count = 0;
    while (count < 128 && s[count]) ++count;
    return count > 0 && count < 128;
}

/* Modules are trusted native code. Bounds below limit metadata iteration, not
 * invalid native pointer access; this tool is not a sandbox. */
static int bounded_text(const char *s, size_t limit, int empty) {
    if (!s || (!empty && !s[0])) return 0;
    for (size_t i = 0; i <= limit; ++i) if (!s[i]) return 1;
    return 0;
}
static int token(const char *s) {
    if (!bounded_text(s, 95, 0)) return 0;
    for (; *s; ++s) if (!((*s >= 'a' && *s <= 'z') ||
        (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') ||
        *s == '_' || *s == '.' || *s == '-')) return 0;
    return 1;
}

/* Failed negotiations must leave even the announced size byte-for-byte intact. */
#define CHECK_NEGOTIATION(entry, type, version) do { \
    type probe, before; \
    memset(&probe, 0xA5, sizeof(probe)); \
    probe.struct_size = sizeof(probe); memcpy(&before, &probe, sizeof(probe)); \
    if ((entry)((version) + 1u, &probe) || memcmp(&probe, &before, sizeof(probe))) \
        return "optional extension writes on rejected version"; \
    probe.struct_size = sizeof(probe) - 1u; memcpy(&before, &probe, sizeof(probe)); \
    if ((entry)((version), &probe) || memcmp(&probe, &before, sizeof(probe))) \
        return "optional extension accepts or writes a small structure"; \
    if ((entry)((version), NULL)) return "optional extension accepts NULL output"; \
} while (0)

static const char *inspect_options(module_handle module, void *instance) {
    void *symbol = lookup_symbol(module, CORVUS_OPTIONS_ENTRYPOINT);
    if (!symbol) { puts("Options: absent (legacy module supported)"); return NULL; }
    corvus_get_options_api_fn entry = NULL;
    memcpy(&entry, &symbol, sizeof(entry));
    CHECK_NEGOTIATION(entry, corvus_options_api, CORVUS_OPTIONS_API_VERSION);
    struct { corvus_options_api api; uint64_t guard; } output = {{0}, UINT64_C(0x123456789)};
    output.api.struct_size = sizeof(output.api);
    if (!entry(CORVUS_OPTIONS_API_VERSION, &output.api) ||
        output.guard != UINT64_C(0x123456789) ||
        output.api.struct_size < sizeof(output.api) ||
        output.api.api_version != CORVUS_OPTIONS_API_VERSION)
        return "invalid options negotiation";
    corvus_options_api *api = &output.api;
    if (!api->count || !api->describe || !api->get_value || !api->set_value)
        return "incomplete options API";
    uint32_t count = api->count(instance);
    if (count > 64) return "too many options";
    const char *keys[64];
    for (uint32_t i = 0; i < count; ++i) {
        corvus_core_option option = {.struct_size = sizeof(option)};
        if (!api->describe(instance, i, &option) || option.struct_size < sizeof(option) ||
            !token(option.key) || !bounded_text(option.label, 255, 0) ||
            !bounded_text(option.description, 255, 1) || !option.values ||
            !option.value_count || option.value_count > 64 ||
            option.default_index >= option.value_count ||
            (option.flags & ~CORVUS_OPTION_RESTART_REQUIRED))
            return "invalid option metadata";
        for (uint32_t j = 0; j < i; ++j) if (!strcmp(keys[j], option.key))
            return "duplicate option key";
        keys[i] = option.key;
        for (uint32_t j = 0; j < option.value_count; ++j) {
            const corvus_option_value *value = &option.values[j];
            if (value->struct_size < sizeof(*value) || !token(value->id) ||
                !bounded_text(value->label, 255, 0)) return "invalid option value";
            for (uint32_t k = 0; k < j; ++k)
                if (!strcmp(option.values[k].id, value->id)) return "duplicate value ID";
        }
        uint32_t selected = api->get_value(instance, option.key);
        if (selected != option.default_index ||
            api->set_value(instance, option.key, option.value_count) ||
            api->get_value(instance, option.key) != selected)
            return "invalid initial option selection or non-atomic rejection";
    }
    printf("Options: %u\n", count);
    return NULL;
}

static const char *inspect_subsystems(module_handle module, void *instance) {
    void *symbol = lookup_symbol(module, CORVUS_SUBSYSTEM_ENTRYPOINT);
    if (!symbol) { puts("Subsystems: absent (legacy module supported)"); return NULL; }
    corvus_get_subsystem_api_fn entry = NULL;
    memcpy(&entry, &symbol, sizeof(entry));
    CHECK_NEGOTIATION(entry, corvus_subsystem_api, CORVUS_SUBSYSTEM_API_VERSION);
    struct { corvus_subsystem_api api; uint64_t guard; } output = {{0}, UINT64_C(0x123456789)};
    output.api.struct_size = sizeof(output.api);
    if (!entry(CORVUS_SUBSYSTEM_API_VERSION, &output.api) ||
        output.guard != UINT64_C(0x123456789) ||
        output.api.struct_size < sizeof(output.api) ||
        output.api.api_version != CORVUS_SUBSYSTEM_API_VERSION)
        return "invalid subsystem negotiation";
    corvus_subsystem_api *api = &output.api;
    if (!api->count || !api->describe || !api->load || !api->capabilities)
        return "incomplete subsystem API";
    int any = api->battery_size || api->battery_dirty || api->battery_copy || api->battery_acknowledge;
    int all = api->battery_size && api->battery_dirty && api->battery_copy && api->battery_acknowledge;
    if (any != all) return "partial per-slot battery group";
    uint32_t capabilities = api->capabilities(instance);
    if (capabilities) return "unloaded instance has session capabilities";
    uint32_t count = api->count();
    if (count > 16) return "too many subsystems";
    const char *ids[16];
    for (uint32_t i = 0; i < count; ++i) {
        corvus_subsystem_info info = {.struct_size = sizeof(info)};
        if (!api->describe(i, &info) || info.struct_size < sizeof(info) ||
            !token(info.id) || !bounded_text(info.label, 255, 0) ||
            !info.num_players || info.num_players > 4 ||
            !info.game_count || info.game_count > 4 || !info.games)
            return "invalid subsystem metadata";
        for (uint32_t j = 0; j < i; ++j) if (!strcmp(ids[j], info.id))
            return "duplicate subsystem ID";
        ids[i] = info.id;
        for (uint32_t j = 0; j < info.game_count; ++j) {
            const corvus_subsystem_rom *rom = &info.games[j];
            if (rom->struct_size < sizeof(*rom) || !bounded_text(rom->label, 255, 0) ||
                !bounded_text(rom->extensions, 255, 0) || rom->required > 1)
                return "invalid subsystem content metadata";
        }
    }
    printf("Subsystems: %u\n", count);
    return NULL;
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
    void *symbol = lookup_symbol(module, CORVUS_CORE_ENTRYPOINT);
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
    const char *extension_error = inspect_options(module, instance);
    if (!extension_error) extension_error = inspect_subsystems(module, instance);
    api->destroy(instance);
    if (extension_error) return fail(module, extension_error);
    printf("PASS CorvusAPI %u: %s (%s), system=%s, formats=%s, state=%s, battery=%s\n",
           api->api_version, info.name, info.id, info.system_name, info.extensions,
           full_state ? "yes" : "no", full_battery ? "yes" : "no");
    close_module(module);
    return 0;
}
