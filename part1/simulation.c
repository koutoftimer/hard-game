#include "simulation.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../print.h"

float const MIN_DISTANCE = 100.;
float const MAX_DISTANCE = 1000.;
float const ASTEROID_HP = 30'000.;
float const BEAM_POWER = 10.;
float const ASTEROID_SPEED = 2.;
float const SHIP_SPEED = 0.;
float const ALLOWED_DISTANCE = 25.;

state_t state;

float rand_float(float from, float to) {
        return from + (1. * rand() / RAND_MAX) * to;
}

Vec3 vec3_diff(Vec3 a, Vec3 b) {
        return (Vec3){
            .x = b.x - a.x,
            .y = b.y - a.y,
            .z = b.z - a.z,
        };
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
void change_focus_distance(Vec3 focus) { state.focus = focus; }

TrueObject random_object() {
        TrueObject res = {.hp = ASTEROID_HP};

        Object *const obj = &res.o;
        obj->location.x = rand_float(MIN_DISTANCE, MAX_DISTANCE);
        obj->location.y = rand_float(MIN_DISTANCE, MAX_DISTANCE);
        float distance = sqrtf(obj->location.x * obj->location.x +
                               obj->location.y * obj->location.y);
        float true_distance = rand_float(MIN_DISTANCE, MAX_DISTANCE);
        obj->location.z = true_distance * true_distance - distance * distance;
        if (obj->location.z > 0) {
                obj->location.z = sqrtf(obj->location.z);
        }
        if (rand() < RAND_MAX / 2) {
                obj->location.z = -obj->location.z;
        }

        res.speed = (Vec3){
            state.location.x - obj->location.x,
            state.location.y - obj->location.y,
            state.location.z - obj->location.z,
        };
        float const mult =
            ASTEROID_SPEED /
            fmax(fmax(fabs(res.speed.x), fabs(res.speed.y)), fabs(res.speed.z));
        res.speed.x *= mult;
        res.speed.y *= mult;
        res.speed.z *= mult;
        return res;
}

void init_state(core_t const *core) {
        srand(100500);

        state.c = *core;
        state.location = (Vec3){0};
        float const SPEED_MULT = SHIP_SPEED;
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

void call_core_process(core_t const *const core) {
        State api = {
            .change_focus_distance = change_focus_distance,
            .get_nearest = get_nearest,
            .get_current_locaton = get_current_locaton,
        };
        core->process(&api);
}

bool form_a_line(size_t n, Vec3 points[n]) {
        if (n < 2) {
                return false;
        }

        Vec3 vectors[n - 1];
        Vec3 *next = vectors;
        for (size_t i = 1; i < n; ++i) {
                // skip points that are the same with points[0]
                bool same = 0 == strncmp((char *)&points[i], (char *)&points[0],
                                         sizeof(Vec3));
                if (same) continue;
                *next++ = vec3_diff(points[i], points[0]);
        }

        for (size_t i = 1; i < next - vectors; ++i) {
                Vec3 const a = vectors[i - 1], b = vectors[i];
                if (a.x == 0 && a.y == 0 && a.z == 0) {
                        continue;
                }
                if (a.x == 0 || a.y == 0 || a.z == 0) {
                        return false;
                }
                float const eps = 1e5;
                float const xx = b.x / a.x;
                float const yy = b.y / a.y;
                float const zz = b.z / a.z;
                if (fabs(xx - yy) > eps || fabs(xx - zz) > eps) {
                        return false;
                }
        }

        return true;
}

void beam_targeted_object(TrueObject *const object) {
        debug("~beam_targeted_object()\n");
        float const mult = 2 * vec3_distance(state.location, state.focus) /
                           vec3_distance(state.location, object->o.location);
        object->hp -= mult * BEAM_POWER;
}

TrueObject *const select_beam_target() {
        debug("~select_beam_target()\n");
        TrueObject *target = nullptr;
        for (size_t i = 0; i < state.objects_len; ++i) {
                TrueObject *object = &state.objects[i];
                if (!form_a_line(3, (Vec3[]){state.location, object->o.location,
                                             state.focus})) {
                        continue;
                }
                if (target == nullptr ||
                    (vec3_distance(target->o.location, state.location) >
                     vec3_distance(object->o.location, state.location))) {
                        target = object;
                }
        }
        return target;
}

void update_state() {
        debug("~update_state()\n");
        // select asteroid/target for focused beam
        TrueObject *target = select_beam_target();
        if (target == nullptr) {
                goto skip_beaming;
        }
        beam_targeted_object(target);
        if (target->hp >= 0.0) {
                goto skip_beaming;
        }
        // remove destroyed asteroid
        for (size_t i = target - state.objects + 1; i < state.objects_len;
             ++i) {
                state.objects[i - 1] = state.objects[i];
        }
        state.objects_len--;
skip_beaming:

        // move objects
        for (size_t i = 0; i < state.objects_len; ++i) {
                TrueObject *object = &state.objects[i];
                object->o.location.x += object->speed.x;
                object->o.location.y += object->speed.y;
                object->o.location.z += object->speed.z;
        }
        state.location.x += state.speed.x;
        state.location.y += state.speed.y;
        state.location.z += state.speed.z;
}

bool collide() {
        for (size_t i = 0; i < state.objects_len; ++i) {
                TrueObject *object = &state.objects[i];
                if (vec3_distance(object->o.location, state.location) <
                    ALLOWED_DISTANCE) {
                        return true;
                }
        }
        return false;
}

bool simulate(core_t const *const core) {
        init_state(core);

        int ticks_to_survive = 1 << 20;
        while (ticks_to_survive--) {
                call_core_process(core);
                update_state();
                if (collide()) {
                        return false;
                }
        }

        return true;
}
