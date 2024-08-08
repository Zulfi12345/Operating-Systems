/*********************************************************************
Program : miniShell Version : 1.3
--------------------------------------------------------------------
skeleton code for linix/unix/minix command line interpreter
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
char line[NL]; /* command input buffer */

typedef struct BackgroundJob
{
  int job_id;
  pid_t pid;
  char command[NL];
  struct BackgroundJob *next;
} BackgroundJob;

BackgroundJob *job_list = NULL;
int job_count = 1;

/*
shell prompt
*/

void prompt(void)
{
  fflush(stdout);
}

void add_background_job(pid_t pid, const char *command)
{
  BackgroundJob *new_job = (BackgroundJob *)malloc(sizeof(BackgroundJob));
  if (!new_job)
  {
    perror("malloc");
    exit(1);
  }
  new_job->job_id = job_count++;
  new_job->pid = pid;
  strncpy(new_job->command, command, NL);
  new_job->next = job_list;
  job_list = new_job;

  printf("[%d] %d\n", new_job->job_id, pid);
}

void remove_background_job(pid_t pid)
{
  BackgroundJob **current = &job_list;
  while (*current)
  {
    BackgroundJob *job = *current;
    if (job->pid == pid)
    {
      printf("[%d]+ Done %s\n", job->job_id, job->command);
      *current = job->next;
      free(job);
      return;
    }
    current = &job->next;
  }
}

void handle_sigchld(int sig)
{
  int saved_errno = errno;
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    remove_background_job(pid);
  }

  errno = saved_errno;
}

int main(int argk, char *argv[], char *envp[])
/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
{
  int frkRtnVal; /* value returned by fork sys call */
  // int wpid;            /* value returned by wait */
  char *v[NV];         /* array of pointers to command line tokens */
  char *sep = " \t\n"; /* command line token separators */
  int i;               /* parse index */
  // int job_id = 1;

  /* Set up signal handler for SIGCHLD to handle background process termination */
  struct sigaction sa;
  sa.sa_handler = &handle_sigchld;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  if (sigaction(SIGCHLD, &sa, NULL) == -1)
  {
    perror("sigaction");
    exit(1);
  }

  /* prompt for and process one command line at a time */
  while (1)
  { /* do Forever */
    prompt();
    fgets(line, NL, stdin);
    fflush(stdin);
    if (feof(stdin))
    { /* non-zero on EOF */
      exit(0);
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
      continue; /* to prompt */
    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++)
    {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL)
        break;
    }

    /* assert i is number of tokens + 1 */
    int background = 0;
    if (i > 1 && strcmp(v[i - 1], "&") == 0)
    {
      background = 1;
      v[i - 1] = NULL;
    }

    if (i > 0 && strcmp(v[0], "cd") == 0)
    {
      if (i > 1)
      {
        // Change to the directory specified in v[1]
        if (chdir(v[1]) != 0)
        {
          perror("cd");
        }
      }
      continue;
    }

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork())
    {
    case -1: /* fork returns error to parent process */
    {
      break;
    }
    case 0: /* code executed only by child process */
    {
      execvp(v[0], v);
    }
    default: /* code executed only by parent process */
    {
      // if (background)
      // {
      //   printf("[%d] %d\n", job_id, frkRtnVal);
      //   job_id++;
      // }
      // else
      // {
      //   // wpid = waitpid(frkRtnVal, NULL, 0);
      // }
      // // wpid = wait(0);
      // break;

      // if (background)
      // {
      //   add_background_job(frkRtnVal, v[0]);
      // }

      if (background)
      {
        // Construct the full command string
        char full_command[NL] = "";
        for (int j = 0; j < i - 1; j++)
        {
          strcat(full_command, v[j]);
          strcat(full_command, " ");
        }
        full_command[strlen(full_command) - 1] = '\0'; // Remove trailing space
        add_background_job(frkRtnVal, full_command);   // Pass the full command
      }
    }
    } /* switch */

  } /* while */

  return 0;
} /* main */
