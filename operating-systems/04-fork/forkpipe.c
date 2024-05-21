#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define RET_SUCCESS 0
#define RET_CHILD_ERROR 1
#define RET_SYSTEM_CALL_ERROR 2
#define N_CHILDS 2
#define NSD_FILENAME "nsd"

int main(int argc, char *argv[]) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fprintf(stderr, "Can't create pipe\n");
        exit(RET_SYSTEM_CALL_ERROR);
    }

    pid_t pidgen = -1;
    pid_t pidnsd = -1;
    if ((pidgen = fork()) == -1) {
        fprintf(stderr, "Error while creating a GEN child process\n");
        exit(RET_SYSTEM_CALL_ERROR);
    }
    if (pidgen != 0 && (pidnsd = fork()) == -1) {
        fprintf(stderr, "Error while creating an NSD child process\n");
        exit(RET_SYSTEM_CALL_ERROR);
    }
    
    if (pidgen != 0 && pidnsd != 0) {
        // MAIN
        close(pipefd[0]);
        close(pipefd[1]);

        sleep(5);
        
        if (kill(pidgen, SIGTERM) != 0) {
            fprintf(stderr, "Error while killing a GEN child process\n");
            exit(RET_SYSTEM_CALL_ERROR);
        }
        fprintf(stderr, "GEN TERMINATED\n");

        bool child_failed = false;
        for (int i = 0; i < N_CHILDS; i++) {
            int wstatus;
            pid_t wpid;

            if ((wpid = wait(&wstatus)) == -1 || WEXITSTATUS(wstatus) != 0) {
                child_failed = true;
            }
        }
        
        if (!child_failed) {
            printf("OK\n");
        } else {
            printf("ERROR\n");
            exit(RET_CHILD_ERROR);
        }
    } 
    else if (pidgen == 0) {
        // GEN
        if (dup2(pipefd[1], STDOUT_FILENO) == -1 || close(pipefd[0]) != 0 || close(pipefd[1]) != 0) {
            exit(RET_SYSTEM_CALL_ERROR);
        }

        srand(time(NULL));

        while (true) {
            printf("%d %d\n", rand() % 4096, rand() % 4096);
            fflush(stdout); 
            sleep(1);
        }

        if (close(STDOUT_FILENO) != 0) {
            fprintf(stderr, "Error while closing writing end of pipe in GEN\n");
            exit(RET_SYSTEM_CALL_ERROR);
        }
    } 
    else if (pidnsd == 0) {
        // NSD
        if (dup2(pipefd[0], STDIN_FILENO) == -1 || close(pipefd[0]) != 0 || close(pipefd[1]) != 0) {
            exit(RET_SYSTEM_CALL_ERROR);
        }

        char nsdexe[255];
        getcwd(nsdexe, 255);
        strcat(nsdexe,"/");
        strcat(nsdexe, NSD_FILENAME);

        if (execl(nsdexe, NSD_FILENAME, (char *)NULL) == -1) {
            fprintf(stderr, "Error while running nsd executable\n");
            exit(RET_SYSTEM_CALL_ERROR);
        }

        if (close(STDIN_FILENO) != 0) {
            fprintf(stderr, "Error while closing reading end of pipe in NSD\n");
            exit(RET_SYSTEM_CALL_ERROR);
        }
    }

    return RET_SUCCESS;
}
