#include "simulation.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../print.h"

#define M_PI 3.14159265358979323846 /* pi */

float const MIN_DISTANCE = 100.;
float const MAX_DISTANCE = 1000.;
float const ASTEROID_HP = 30'000.;
float const BEAM_POWER = 10.;
float const ASTEROID_SPEED = 2.;
float const SHIP_SPEED = 0.;
float const ALLOWED_DISTANCE = 25.;
int const SIMULATION_TICKS = 1e4;

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

        Asteroid const *nearest = state.objects;
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

Vec3 random_point_on_a_sphere(float const radius) { return (Vec3){0}; }

Asteroid random_object(size_t const orbit) {
        int const steps_to_destroy_asteroid =
            (int)(1 + ASTEROID_HP / BEAM_POWER);

        float const safe_radius =
            orbit * steps_to_destroy_asteroid * ASTEROID_SPEED +
            ASTEROID_SPEED * ceilf(ALLOWED_DISTANCE / ASTEROID_SPEED);

        Asteroid res = {
            .hp = ASTEROID_HP,
            .o.location = random_point_on_a_sphere(safe_radius),
        };

        Vec3 const distance_vec = {
            state.location.x - res.o.location.x,
            state.location.y - res.o.location.y,
            state.location.z - res.o.location.z,
        };
        float const distance = sqrtf(distance_vec.x * distance_vec.x +
                                     distance_vec.y * distance_vec.y +
                                     distance_vec.z * distance_vec.z);
        float const factor = ASTEROID_SPEED / distance;
        res.speed.x = distance_vec.x * factor;
        res.speed.y = distance_vec.y * factor;
        res.speed.z = distance_vec.z * factor;
        return res;
}

void init_state(core_t const *core) {
        /* srand(100500); */
        srand((unsigned int)time(nullptr));

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
                state.objects[i] = random_object(i + 1);
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
                float const eps = 1e-5;
                float const xx = b.x / a.x;
                float const yy = b.y / a.y;
                float const zz = b.z / a.z;
                if (fabs(xx - yy) > eps || fabs(xx - zz) > eps) {
                        return false;
                }
        }

        return true;
}

float sphere_volume(float const radius) {
        return 4. / 3. * M_PI * radius * radius * radius;
}

float cilider_volume(float const height, float const radius) {
        return height * M_PI * radius * radius;
}

float cone_volume(float const height, float const radius) {
        return 1. / 3. * M_PI * height * radius * radius;
}

float cut_cone_volume(float const height, float const r, float const R) {
        return 1. / 3. * M_PI * height * (r * r + r * R + R * R);
}

void beam_targeted_object(Asteroid *const object) {
        debug("~beam_targeted_object()\n");
        float const focus = vec3_distance(state.location, state.focus);
        float const asteroid =
            vec3_distance(state.location, object->o.location);

        float mult;
        float const beam_radius = 1.;
        float const asteroid_radius = 1.;
        float const asteroid_volume = sphere_volume(asteroid_radius);

        if (focus * 2 < asteroid) {  // before first half
                float const radius = beam_radius * (asteroid / focus - 1);
                float const beam_square = M_PI * radius * radius;
                float const asteroid_square =
                    M_PI * asteroid_radius * asteroid_radius;
                mult = asteroid_square / beam_square;

        } else if (focus < asteroid - asteroid_radius) {  // after first half
                float const r =
                    beam_radius * (asteroid - focus - asteroid_radius) / focus;
                float const R =
                    beam_radius * (asteroid - focus + asteroid_radius) / focus;
                float const beam_volume =
                    cut_cone_volume(2 * asteroid_radius, r, R);
                mult = beam_volume / asteroid_volume;

        } else if (focus < asteroid + asteroid_radius) {  // inside
                float const radius = beam_radius * asteroid_radius / asteroid;
                float const beam_volume =
                    2. * cone_volume(asteroid_radius, radius);
                mult = asteroid_volume / beam_volume;

        } else {  // far behind
                float const r =
                    beam_radius * (focus - asteroid - asteroid_radius) / focus;
                float const R =
                    beam_radius * (focus - asteroid + asteroid_radius) / focus;
                float const beam_volume =
                    cut_cone_volume(asteroid_radius, r, R);
                mult = beam_volume / asteroid_volume;
        }

        debug("~beam_targeted_object(): mult=%f\n", mult);

        object->hp -= mult * BEAM_POWER;
}

Asteroid *const select_beam_target() {
        debug("~select_beam_target()\n");
        Asteroid *target = nullptr;
        for (size_t i = 0; i < state.objects_len; ++i) {
                Asteroid *object = &state.objects[i];
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
        Asteroid *target = select_beam_target();
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
                Asteroid *object = &state.objects[i];
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
                Asteroid *object = &state.objects[i];
                if (vec3_distance(object->o.location, state.location) <
                    ALLOWED_DISTANCE) {
                        return true;
                }
        }
        return false;
}

bool simulate(core_t const *const core) {
        init_state(core);

        int ticks_to_survive = SIMULATION_TICKS;
        while (ticks_to_survive--) {
                call_core_process(core);
                update_state();
                if (collide()) {
                        return false;
                }
        }

        return true;
}
