#include "simulation.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

state_t state;
float const OBJECT_SPEED = 0.0;
float const MIN_DISTANCE = 0.0;
float const MAX_DISTANCE = 0.0;

float rand_float(float from, float to) {
        return from + (1. * rand() / RAND_MAX) * to;
}

float vec3_distance(Vec3 const a, Vec3 const b) {
        float const xx = a.x - b.x;
        float const yy = a.y - b.y;
        float const zz = a.z - b.z;
        return sqrt(xx * xx + yy * yy + zz * zz);
}

Object *get_nearest() {
        if (state.objects_len < 1) {
                return nullptr;
        }

        TrueObject const *nearest = state.objects;
        for (size_t i = 1; i < state.objects_len; ++i) {
                float const distance_to_object =
                    vec3_distance(*(Vec3 *)&state.objects[i].o, state.location);
                float const distance_to_nearest =
                    vec3_distance(*(Vec3 *)&state.objects[i].o, state.location);
                if (distance_to_object < distance_to_nearest) {
                        nearest = &state.objects[i];
                }
        }

        // don't leak actual address to the player
        Object *res = malloc(sizeof(Object));
        if (res == nullptr) {
                fprintf(stderr, "%s:%d - Can't allocate memory for Object",
                        __FILE__, __LINE__);
                exit(1);
        }
        *res = nearest->o;
        return res;
}

Vec3 get_current_locaton() { return state.location; }
void change_direction(Vec3 focus) { state.focus = focus; }

TrueObject random_object() {
        TrueObject res = {0};

        Object *const obj = &res.o;
        obj->location.x = rand_float(MIN_DISTANCE, MAX_DISTANCE);
        obj->location.y = rand_float(MIN_DISTANCE, MAX_DISTANCE);
        float distance = sqrtf(obj->location.x * obj->location.x +
                               obj->location.y * obj->location.y);
        float true_distance = rand_float(MIN_DISTANCE, MAX_DISTANCE);
        // TODO: ensure we aren't taking sqrt out of negative
        obj->location.z =
            sqrtf(true_distance * true_distance - distance * distance);
        if (rand() < RAND_MAX / 2) {
                obj->location.z = -obj->location.z;
        }

        float const SPEED_MULT = 1.;
        res.speed = (Vec3){
            rand_float(-SPEED_MULT, SPEED_MULT),
            rand_float(-SPEED_MULT, SPEED_MULT),
            rand_float(-SPEED_MULT, SPEED_MULT),
        };

        return res;
}

void init_state(core_t const *core) {
        srand((unsigned int)time(nullptr));

        state.c = *core;
        state.location = (Vec3){0};
        float const SPEED_MULT = 1.;
        state.speed = (Vec3){
            rand_float(-SPEED_MULT, SPEED_MULT),
            rand_float(-SPEED_MULT, SPEED_MULT),
            rand_float(-SPEED_MULT, SPEED_MULT),
        };
        state.objects_len = 1 << 10;
        for (int i = 0; i < state.objects_len; ++i) {
                state.objects[i] = random_object();
        }
        return;
}

bool simulate(core_t const *const core) {
        init_state(core);

        int ticks_to_survive = 1 << 20;
        while (ticks_to_survive--) {
                State api = {
                    .change_direction = change_direction,
                    .get_nearest = get_nearest,
                    .get_current_locaton = get_current_locaton,
                };
                core->process(&api);
        }

        return true;
}
