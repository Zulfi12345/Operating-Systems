#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20  /* max number of command tokens */
#define NL 100 /* input buffer size */
char line[NL]; /* command input buffer */

typedef struct
{
    int job_id;
    pid_t pid;
    char full_command[NL];
    int completed; /* new field to track completion */
} BackgroundJob;

BackgroundJob jobs[NV];
int job_count = 0;

void prompt(void)
{
    fflush(stdout);
}

void check_background_jobs()
{
    int status;
    pid_t pid;
    for (int i = 0; i < job_count; i++)
    {
        if (!jobs[i].completed && (pid = waitpid(jobs[i].pid, &status, WNOHANG)) > 0)
        {
            jobs[i].completed = 1;
        }
    }
}

void print_done_jobs()
{
    for (int i = 0; i < job_count; i++)
    {
        if (jobs[i].completed)
        {
            printf("[%d]+ Done %s\n", jobs[i].job_id, jobs[i].full_command);
            // Remove the job from the list
            for (int j = i; j < job_count - 1; j++)
            {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            i--; // Adjust index due to shift
        }
    }
}

int main(int argk, char *argv[], char *envp[])
{
    int frkRtnVal;
    char *v[NV];
    char *sep = " \t\n";
    int i;
    int job_id = 1;

    while (1)
    {
        prompt();
        fgets(line, NL, stdin);
        fflush(stdin);
        if (feof(stdin))
        {
            exit(0);
        }
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

        check_background_jobs();
        print_done_jobs();

        switch (frkRtnVal = fork())
        {
        case -1:
        {
            perror("fork");
            break;
        }
        case 0:
        {
            execvp(v[0], v);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        default:
        {
            if (background)
            {
                BackgroundJob new_job;
                new_job.job_id = job_id++;
                new_job.pid = frkRtnVal;
                new_job.completed = 0; /* initially not completed */
                // Save the full command
                strcpy(new_job.full_command, line);
                // Remove the trailing '&' and newline from the command
                new_job.full_command[strcspn(new_job.full_command, "&")] = '\0';
                new_job.full_command[strcspn(new_job.full_command, "\n")] = '\0';
                jobs[job_count++] = new_job;
                printf("[%d] %d\n", new_job.job_id, new_job.pid);
            }
            else
            {
                waitpid(frkRtnVal, NULL, 0);
            }
            break;
        }
        }

        check_background_jobs();
        print_done_jobs();
    }

    return 0;
}
