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
// fprintf(stdout, "out");
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
{
  int frkRtnVal;
  char *v[NV];
  char *sep = " \t\n";
  int i;

  /* Set up signal handler for SIGCHLD to handle background process termination */
  signal(SIGCHLD, handle_sigchld);

  /* prompt for and process one command line at a time */
  while (1)
  {
    prompt();
    if (fgets(line, NL, stdin) == NULL)
    {
      if (feof(stdin))
      {
        exit(0);
      }
      else
      {
        perror("fgets");
        continue;
      }
    }
    fflush(stdin);
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
      continue;
    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++)
    {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL)
        break;
    }

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
        if (chdir(v[1]) != 0)
        {
          perror("cd");
        }
      }
      continue;
    }

    switch (frkRtnVal = fork())
    {
    case -1:
      perror("fork");
      break;
    case 0:
      execvp(v[0], v);
      perror("execvp");
      exit(1);
    default:
      if (background)
      {
        char full_command[NL] = "";
        for (int j = 0; j < i - 1; j++)
        {
          strcat(full_command, v[j]);
          strcat(full_command, " ");
        }
        full_command[strlen(full_command) - 1] = '\0';
        add_background_job(frkRtnVal, full_command);
      }
    }
  }

  return 0;
}
