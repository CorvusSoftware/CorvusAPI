// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

/* Trusted negative fixtures: a valid legacy base plus an invalid extension. */
#include "../examples/minimal_core.c"

#if CORVUS_INVALID_FIXTURE == 1
/* Rejected negotiation must not modify the caller's bytes. */
CORVUS_API_EXPORT int corvus_get_options_api(uint32_t version, corvus_options_api *out) {
    (void)version;
    if (out) out->api_version = 77;
    return 0;
}
#elif CORVUS_INVALID_FIXTURE == 2
static uint32_t invalid_count(void) { return 0; }
static int invalid_describe(uint32_t index, corvus_subsystem_info *out) {
    (void)index; (void)out; return 0;
}
static int invalid_load(void *core, const char *id, const corvus_game *games, size_t count) {
    (void)core; (void)id; (void)games; (void)count; return 0;
}
static uint32_t invalid_caps(void *core) { (void)core; return 0; }
static size_t invalid_battery_size(void *core, uint32_t slot) {
    (void)core; (void)slot; return 0;
}
CORVUS_API_EXPORT int corvus_get_subsystem_api(uint32_t version, corvus_subsystem_api *out) {
    if (version != CORVUS_SUBSYSTEM_API_VERSION || !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_subsystem_api api = {
        .struct_size = sizeof(api), .api_version = CORVUS_SUBSYSTEM_API_VERSION,
        .count = invalid_count, .describe = invalid_describe, .load = invalid_load,
        .capabilities = invalid_caps, .battery_size = invalid_battery_size
    };
    *out = api; /* Deliberately incomplete per-slot battery group. */
    return 1;
}
#elif CORVUS_INVALID_FIXTURE == 3
static uint32_t invalid_count(void *core) { (void)core; return 65; }
static int invalid_describe(void *core, uint32_t index, corvus_core_option *out) {
    (void)core; (void)index; (void)out; return 0;
}
static uint32_t invalid_get(void *core, const char *key) {
    (void)core; (void)key; return UINT32_MAX;
}
static int invalid_set(void *core, const char *key, uint32_t value) {
    (void)core; (void)key; (void)value; return 0;
}
CORVUS_API_EXPORT int corvus_get_options_api(uint32_t version, corvus_options_api *out) {
    if (version != CORVUS_OPTIONS_API_VERSION || !out || out->struct_size < sizeof(*out)) return 0;
    const corvus_options_api api = {sizeof(api), CORVUS_OPTIONS_API_VERSION,
        invalid_count, invalid_describe, invalid_get, invalid_set};
    *out = api;
    return 1;
}
#endif
