#pragma once

typedef struct {
        float x;
        float y;
        float z;
} Vec3;

typedef struct Object {
        Vec3 locaiton;
} Object;

typedef struct {
        Object* (*get_nearest)(void);
        Vec3 (*get_current_locaton)(void);
        void (*change_direction)(Vec3);
} State;

typedef struct {
        void (*process)(State const* const);
} core_t;

#ifdef IMPLEMENTATION
void process(State const* const);

const core_t core = {
    .process = process,
};
#endif  // IMPLEMENTATION
