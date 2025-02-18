#include "simulation.h"

#include "../print.h"

state_t state;

Object *get_nearest() {
        TODO("unimplemented");
        return nullptr;
}

Vec3 get_current_locaton() {
        TODO("unimplemented");
        return (Vec3){0};
}
void change_direction(Vec3 direction) {
        TODO("unimplemented");
        return;
}

void init_core(core_t const *core) {
        state.c = *core;
        return;
}

bool simulate(core_t const *const core) {
        init_core(core);

        State api = {
            .change_direction = change_direction,
            .get_nearest = get_nearest,
            .get_current_locaton = get_current_locaton,
        };
        core->process(&api);

        return true;
}
