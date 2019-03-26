#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMD_SIZE 128

const char * ARG_PARSER = ":u:p:h";
const char * PATH_DOGDOOR = "/proc/dogdoor";
const char * PATH_DOGDOOR_PID = "/proc/dogdoor_pid";
const char * CMD_ECHO  = "echo %d > ";
const char * CMD_ECHO_1  = "echo -1 > ";

void help_usage() {
    fprintf(stderr, "Usage: bingo -u <USER_NAME> -p <PID>\n");
}
void help_pid() {
    fprintf(stderr, "\tPID: prevent kill PID\n");
}
void help_username() {
    fprintf(stderr, "\tUSER_NAME: logging to /proc/dogdoor_log lastly ten files accessed by USER_NAME\n");
}
void help_msg() {
    help_usage();
    help_username();
    help_pid();
}
void need_help() {
    fprintf(stderr, "enter './bingo -h' for help message\n");
}

void process_username(char * username) {
    struct passwd * pw;
    char cmd[CMD_SIZE];
    char * p_cmd;

    pw = getpwnam(username);
    if (pw != NULL) {
        p_cmd = (char *)malloc(strlen(CMD_ECHO) + strlen(PATH_DOGDOOR));
        strcpy(p_cmd, CMD_ECHO);
        strcat(p_cmd, PATH_DOGDOOR);
        sprintf(cmd, p_cmd, pw->pw_uid);
        free(p_cmd);
    }
    else {
        strcpy(cmd, CMD_ECHO_1);
        strcat(cmd, PATH_DOGDOOR);
    }
    system(cmd);
}

void process_pid(int pid) {
    char cmd[CMD_SIZE];
    char * p_cmd = (char *)malloc(strlen(CMD_ECHO) + strlen(PATH_DOGDOOR_PID));
    strcpy(p_cmd, CMD_ECHO);
    strcat(p_cmd, PATH_DOGDOOR_PID);
    sprintf(cmd, p_cmd, pid);
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
            if (opt == 'u') {
                help_usage();
                help_username();
            }
            if (opt == 'p') {
                help_usage();
                help_pid();
            }
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
