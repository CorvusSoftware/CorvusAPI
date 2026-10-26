// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT

#include <corvusapi.h>
#include <achievements.h>

int main(void) {
    corvus_core_api core = {0};
    corvus_achievements_api achievements = {0};
    core.struct_size = sizeof(core);
    achievements.struct_size = sizeof(achievements);
    return CORVUS_API_VERSION != 0u ||
           CORVUS_ACHIEVEMENTS_API_VERSION != 1u ||
           core.struct_size != sizeof(corvus_core_api) ||
           achievements.struct_size != sizeof(corvus_achievements_api);
}
