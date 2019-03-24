#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>

/*const char * PROC_PATH = "/proc/dogdoor";*/
const char * PROC_PATH = "/tmp/test.txt";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: bingo <USER_NAME>\n");
        return 1;
    }

    struct passwd * pw = getpwnam(argv[1]);
    FILE *fp = fopen(PROC_PATH, "w");

    (pw != NULL) ? fprintf(fp, "%d", pw->pw_uid) : fprintf(fp, "-1");

    fclose(fp);
    return 0;
}
