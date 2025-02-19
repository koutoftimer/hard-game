#define IMPLEMENTATION
#include "../strategy.h"
#undef IMPLEMENTATION

#include <dlfcn.h>
#include <errno.h>
#include <git2.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../print.h"
#include "./core.h"
#include "./simulation.h"

char const *const intro_message() {
        static char message[] = {
#embed "./intro.txt"
        };
        return message;
}

static char makefile[] = {
#embed "./Makefile"
};
static char corec[] = {
#embed "./core.c"
};
static char coreh[] = {
#embed "./core.h"
};
static char fail[] = {
#embed "./fail.txt"
};
static char success[] = {
#embed "./success.txt"
};

git_repository *create_repo();
core_t const *const load_core();

void strategy() {
        // create git repo
        git_libgit2_init();
        git_repository *repo = create_repo();
        git_repository_state_t state = git_repository_state(repo);
        (void)state;
        git_repository_free(repo);
        git_libgit2_shutdown();

        // create hints about what to do
        write_file("Makefile", makefile);
        write_file("core.h", coreh);
        // do not overwrite implementation
        if (access("core.c", F_OK) != 0) {
                write_file("core.c", corec);
        }

        animated_print(intro_message());

        core_t const *core = nullptr;
        while ((core = load_core()) == nullptr) {
                animated_print("load <  ");
        }

        if (simulate(core)) {
                printf(CLEAR_SCREEN_ANSI);
                animated_print(success);
        } else {
                animated_print(fail);
        }
}

core_t const *const load_core() {
        char module_name[1 << 20];
        if (scanf("%s", module_name) == -1) {
                fprintf(stderr, "ERROR: %s\n\n", strerror(errno));
                return nullptr;
        }
        FILE *fp = fopen(module_name, "rb");
        if (fp == nullptr) {
                fprintf(stderr, "ERROR: %s\n\n", strerror(errno));
                return nullptr;
        }
        fclose(fp);

        void *dl = dlopen(module_name, RTLD_LAZY);
        if (dl == nullptr) {
                fprintf(stderr, "ERROR: %s\n\n", dlerror());
                return nullptr;
        }

        core_t *export = dlsym(dl, "core");
        if (export == nullptr) {
                fprintf(stderr, "ERROR: %s\n\n", dlerror());
                return nullptr;
        }

        return export;
}

git_repository *create_repo() {
        git_repository *repo = nullptr;
        char const *const cwd = getcwd(nullptr, 0);
        int err = git_repository_init(&repo, cwd, false);

        if (err < 0) {
                git_error const *const error = git_error_last();
                fprintf(stderr, "ERROR: %s\n",
                        error ? error->message : "Unknown error");
                exit(err);
        }

        return repo;
}
