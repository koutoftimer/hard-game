#include <dlfcn.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

#include "strategy.h"

void launch_strategy(char const *const base_dir, int id);

int main(int argc, char *argv[]) {
        char const *const base_dir = dirname(argv[0]);
        for (int i = 0; i < 2; ++i) {
                launch_strategy(base_dir, i);
        }
        return EXIT_SUCCESS;
}

void launch_strategy(char const *const base_dir, int id) {
        char strategy_filename[1024] = {0};
        sprintf(strategy_filename, "%s/part%d/strategy.so", base_dir, id);

        void *dl = dlopen(strategy_filename, RTLD_LAZY);
        if (dl == nullptr) {
                fprintf(stderr, "%s\n", dlerror());
                exit(EXIT_FAILURE);
        }
        strategy_export_t *export = dlsym(dl, "strategy_export");
        if (export == nullptr) {
                fprintf(stderr, "%s\n", dlerror());
                exit(EXIT_FAILURE);
        }
        export->strategy();
}
