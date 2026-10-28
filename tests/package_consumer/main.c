// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

#include <corvusapi.h>
#include <achievements.h>

int main(void) {
    corvus_core_api core = {0};
    corvus_achievements_api achievements = {0};
    corvus_options_api options = {0};
    corvus_subsystem_api subsystems = {0};
    options.struct_size = sizeof(options);
    subsystems.struct_size = sizeof(subsystems);
    core.struct_size = sizeof(core);
    achievements.struct_size = sizeof(achievements);
    return CORVUS_API_VERSION != 0u ||
           CORVUS_ACHIEVEMENTS_API_VERSION != 1u ||
           CORVUS_OPTIONS_API_VERSION != 1u || CORVUS_SUBSYSTEM_API_VERSION != 1u ||
           options.struct_size != sizeof(corvus_options_api) ||
           subsystems.struct_size != sizeof(corvus_subsystem_api) ||
           core.struct_size != sizeof(corvus_core_api) ||
           achievements.struct_size != sizeof(corvus_achievements_api);
}
