#pragma once

#include <stddef.h>

#include "core.h"

typedef struct {
        Object o;
        Vec3 speed;
        float hp;
} TrueObject;

typedef struct {
        core_t c;
        Vec3 location;
        Vec3 speed;
        Vec3 focus;
        TrueObject objects[1024];
        size_t objects_len;
} state_t;

Object *get_nearest();
Vec3 get_current_locaton();
void change_direction(Vec3);
bool simulate(core_t const *const);
