/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20			/* max number of command tokens */
#define NL 100			/* input buffer size */
char line[NL];	/* command input buffer */

/*
	shell prompt
 */

void prompt(void) {
  fprintf(stdout, "\n msh> ");
  fflush(stdout);
}

void handle_background_completion(int sig) {
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    printf("\nProcess %d completed\n", pid);
    prompt();
  }
}

int main(int argk, char *argv[], char *envp[]) {
  int frkRtnVal;	/* value returned by fork sys call */
  int wpid;		/* value returned by wait */
  char *v[NV];	/* array of pointers to command line tokens */
  char *sep = " \t\n";/* command line token separators    */
  int i;		/* parse index */
  int is_background; /* background process flag */

  signal(SIGCHLD, handle_background_completion);

  /* prompt for and process one command line at a time  */
  while (1) {			/* do Forever */
    prompt();
    if (fgets(line, NL, stdin) == NULL) {
      if (feof(stdin)) {		/* non-zero on EOF  */
        fprintf(stderr, "EOF pid %d feof %d ferror %d\n", getpid(), feof(stdin), ferror(stdin));
        exit(0);
      }
      perror("fgets");
      continue;
    }

    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
      continue;			/* to prompt */

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL)
        break;
    }

    is_background = 0;
    if (i > 0 && strcmp(v[i - 1], "&") == 0) {
      is_background = 1;
      v[i - 1] = NULL;
    }

    if (strcmp(v[0], "cd") == 0) {
      if (v[1] == NULL) {
        fprintf(stderr, "cd: missing operand\n");
      } else {
        if (chdir(v[1]) != 0) {
          perror("chdir");
        }
      }
      continue;
    }

    switch (frkRtnVal = fork()) {
      case -1:			/* fork returns error to parent process */
        perror("fork");
        break;
      case 0:			/* code executed only by child process */
        if (execvp(v[0], v) == -1) {
          perror("execvp");
          exit(EXIT_FAILURE); /* Terminate child process if exec fails */
        }
        break;
      default:			/* code executed only by parent process */
        if (is_background) {
          printf("Process %d started in background\n", frkRtnVal);
        } else {
          wpid = waitpid(frkRtnVal, NULL, 0);
          if (wpid == -1) {
            perror("waitpid");
          }
          printf("%s done\n", v[0]);
        }
        break;
    }				/* switch */
  }				/* while */
}				/* main */
