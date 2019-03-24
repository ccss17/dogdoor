#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

#define CMD_SIZE 256

const char * PROC_PATH = "/proc/dogdoor";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: bingo <USER_NAME>\n");
        return 1;
    }

    struct passwd * pw = getpwnam(argv[1]);
    char cmd[CMD_SIZE];

    if (pw != NULL)
        sprintf(cmd, "echo %d > /proc/dogdoor", pw->pw_uid);
    else
        sprintf(cmd, "echo -1 > /proc/dogdoor");
    system(cmd);

    return 0;
}
