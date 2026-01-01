#include <string.h>

char *cleanse(char *input) {
    if (input[0] == '/') {
        input++;
    }

    char *p;
    while ((p = strstr(input, ".")) != NULL) {
        memmove(p, p + 2, strlen(p + 2) + 1);
    }

    while ((p = strstr(input, "//")) != NULL) {
        memmove(p, p + 1, strlen(p + 1) + 1);
    }

    int len = strlen(input);
    if (len > 0 && input[len - 1] == '/') {
        input[len - 1] = '\0';
    }

    return input;
}