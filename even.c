#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Signal handler function
void handle_signal(int signal) {
    if (signal == SIGHUP) {
        printf("Ouch!\n");
    } else if (signal == SIGINT) {
        printf("Yeah!\n");
    }
}

int main(int argc, char *argv[]) {
    // Check if the number of arguments is correct
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        return 1;
    }

    // Convert input argument to integer
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Please provide a positive integer.\n");
        return 1;
    }

    // Register signal handlers
    signal(SIGHUP, handle_signal);
    signal(SIGINT, handle_signal);

    // Print the first n even numbers with a delay
    for (int i = 0; i < n; ++i) {
        printf("%d\n", 2 * i);
        sleep(5);
    }

    return 0;
}
