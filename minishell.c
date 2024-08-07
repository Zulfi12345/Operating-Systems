/*********************************************************************
Program : miniShell Version : 1.3
--------------------------------------------------------------------
skeleton code for linux/unix/minix command line interpreter
--------------------------------------------------------------------
File : minishell.c
Compiler/System : gcc/linux
********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define NV 20  /* max number of command tokens */
#define NL 100 /* input buffer size */
#define MAX_JOBS 100

char line[NL]; /* command input buffer */

/* Job structure to hold job ID and PID */
typedef struct {
    int job_id;
    pid_t pid;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;
int next_job_id = 1;

/* shell prompt */
void prompt(void) {
    fflush(stdout);
}

/* Handle SIGCHLD to report background process completion */
void handle_sigchld(int sig) {
    int saved_errno = errno;
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                printf("\n[%d] Done %d\n", jobs[i].job_id, pid);
                /* Remove the job from the list */
                for (int j = i; j < job_count - 1; j++) {
                    jobs[j] = jobs[j + 1];
                }
                job_count--;
                if (job_count == 0) {
                    next_job_id = 1; // Reset job ID when no background jobs are running
                }
                break;
            }
        }
    }
    errno = saved_errno;
}

int main(int argc, char *argv[], char *envp[]) {
    int frkRtnVal;       /* value returned by fork sys call */
    int wpid;            /* value returned by wait */
    char *v[NV];         /* array of pointers to command line tokens */
    char *sep = " \t\n"; /* command line token separators */
    int i;               /* parse index */

    // // Handle SIGCHLD to report background process completion
    // struct sigaction sa;
    // sa.sa_handler = &handle_sigchld;
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART;
    // if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(1);
    // }

    /* prompt for and process one command line at a time */
    while (1) {
        prompt();
        fgets(line, NL, stdin);
        fflush(stdin);
        if (feof(stdin)) {
            exit(0);
        }
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
            continue; /* to prompt */
        v[0] = strtok(line, sep);
        for (i = 1; i < NV; i++) {
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL)
                break;
        }
        /* assert i is number of tokens + 1 */
        int background = 0;
        if (i > 1 && strcmp(v[i - 1], "&") == 0) {
            background = 1;
            v[i - 1] = NULL; // Remove the '&' token
        }

        if (strcmp(v[0], "cd") == 0) {
            if (v[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else if (chdir(v[1]) != 0) {
                perror("cd");
            }
            continue;
        }

        /* fork a child process to exec the command in v[0] */
        switch (frkRtnVal = fork()) {
        case -1: /* fork returns error to parent process */
            perror("fork");
            break;
        case 0: /* code executed only by child process */
            execvp(v[0], v);
            perror("execvp");
            exit(1);
        default: /* code executed only by parent process */
            if (background) {
                jobs[job_count].job_id = next_job_id++;
                jobs[job_count].pid = frkRtnVal;
                job_count++;
                printf("[%d] %d\n", jobs[job_count - 1].job_id, frkRtnVal);
            } else {
                wpid = waitpid(frkRtnVal, NULL, 0);
            }
            break;
        } /* switch */
    } /* while */
    return 0;
} /* main */
