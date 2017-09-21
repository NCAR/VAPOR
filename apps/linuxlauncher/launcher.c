#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

int main(int argc, char **argv) {
    char path[PATH_MAX];
    readlink("/proc/self/exe", path, PATH_MAX);
    char *p = &path[strlen(path) - 1];
    int up = 2;
    while (p != path && up) {
        if (*p == '/') {
            *p = 0;
            up--;
        }
        p--;
    }

    setenv("VAPOR_HOME", path, 1);

    // TODO Append to path instead of overriding
    strcat(path, "/lib");
    setenv("LD_LIBRARY_PATH", path, 1);

    argv[0] = "@TARGET_NAME@";

    strcat(path, "/@TARGET_NAME@");
    printf("PATH %s\n", path);

    execv(path, argv);
    return 0;
}
