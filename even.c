#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sig_HUP(int n)
{
    printf("Ouch!\n");
    // fflush(stdout);
}

void sig_INT(int n)
{
    printf("Yeah!\n");
    // fflush(stdout);
}

void evenNumbers(int n)
{
    for (int i = 1; i <= n; i++)
    {
        printf("%d\n", i * 2);
        // fflush(stdout);
        sleep(5);
    }
}

int main(void)
{

    printf("Enter n: ");
    int n = 0;
    scanf("%d", &n);

    if(n<0){
        return -1;
    }

    signal(SIGHUP, sig_HUP);
    signal(SIGINT, sig_INT);

    evenNumbers(n);

    return 0;
}