#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int mkdir_r(char *path, mode_t mode) {
    char *slash = strchr(path, '/');
    while (slash != NULL) {
        *slash = '\0';
        if (strlen(path) > 0) {
            if (mkdir(path, mode) != 0) {
                if (errno != EEXIST) {
                    return errno;
                    *slash = '/';
                    break;
                }
            }
        }
        *slash = '/';
        slash = strchr(slash + 1, '/');
    }
    if (errno == 0) {
        if (mkdir(path, mode) != 0) {
            if (errno != EEXIST) {
                return errno;
            }
        }
    }
    return 0;
}