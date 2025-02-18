#pragma once

typedef void (*strategy_t)(void);
typedef struct {
        strategy_t strategy;
} strategy_export_t;

void strategy();

#ifdef IMPLEMENTATION
const strategy_export_t strategy_export = {
    .strategy = strategy,
};
#endif  // IMPLEMENTATION
