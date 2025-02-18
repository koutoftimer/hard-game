#pragma once

#include <stddef.h>

#include "core.h"

typedef struct {
        core_t c;
        Vec3 location;
        Object objects[1024];
        size_t objects_len;
} state_t;

Object *get_nearest();
Vec3 get_current_locaton();
void change_direction(Vec3);
bool simulate(core_t const *const);
