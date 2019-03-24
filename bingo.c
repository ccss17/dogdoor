#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>

#define DEBUG 0

#if DEBUG
const char * PROC_PATH = "/tmp/test.txt";
#else
const char * PROC_PATH = "/proc/dogdoor";
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: bingo <USER_NAME>\n");
        return 1;
    }

    struct passwd * pw = getpwnam(argv[1]);
    FILE * fp = fopen(PROC_PATH, "w");

    if (fp == NULL) {
        fprintf(stderr, "%s does not exist!\n", PROC_PATH);
        return 1;
    }

    (pw != NULL) ? fprintf(fp, "%d", pw->pw_uid) : fprintf(fp, "-1");

    fclose(fp);
    return 0;
}
