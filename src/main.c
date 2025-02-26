#include <stdio.h>
#include <signal.h>

#include "../include/container.h"
#include "../include/cli.h"
#include "../include/hashtable.h"

cli_t * cli = NULL;
volatile sig_atomic_t sigint_received = 0;

// this signal handler handles situations where the user iinputs ctrl + z to temporarily stop the program
static void clean_resources(int sig) {
    (void)sig;
    if (NULL != cli->data1) {
        container_cleanup(cli);
    }
    if (cli->command) {
        free(cli->command);
        cli->command = NULL;
    }
    free(cli);
    exit(0);
}

// this signal handler handes ctrl + c and I need to make sure that all resources are freed before the exit
void sigint_handler(int sig) {
    // If the input buffer is allocated, free it before exiting
    if (cli->command != NULL) {
        free(cli->command);
        cli->command = NULL;
        printf("\nInput buffer freed. Program exiting due to SIGINT...\n");
    }
    exit(0);
}

int main(void) {
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal setup failed");
    }
    if (1 != sigint_received) {
        signal(SIGTERM, clean_resources);
    }

    //multithread later to make pings more optimal

    //signal(SIGTSTP, clean_resources); using ctrl z and pauses application
    cli = cli_initialize();
    cli_start(cli, container_commands);
    return 0;
}

