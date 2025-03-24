#include <stdio.h>
#include <signal.h>

#include "../include/container.h"
#include "../include/cli.h"
#include "../include/hashtable.h"

cli_t * cli = NULL;
volatile sig_atomic_t exit_flag = 0;

// this signal handler handles situations where the user iinputs ctrl + z to temporarily stop the program
static void clean_resources(int sig) {
    exit_flag = 1; 
    printf("Sig: %d.\n", sig);
    container_cleanup(cli);
    if (cli->command) {
        free(cli->command);
        cli->command = NULL;
    }
    cli_free_arguments(cli);
    free(cli);
    cli = NULL;
    exit(0);
}

// this signal handler handes ctrl + c and I need to make sure that all resources are freed before the exit
void sigint_handler(int sig) {
    // If the input buffer is allocated, free it before exiting
    //if (cli->command != NULL) {
    //    free(cli->command);
    //    cli->command = NULL;
    //    printf("\nInput buffer freed. Program exiting due to SIGINT...\n");
    //}
    exit_flag = 1; 
    if (NULL != cli) {
        printf("Sig: %d.\n", sig);
        container_cleanup(cli);
        if (NULL != cli->command) {
            free(cli->command);
            cli->command = NULL;
        }
        cli_free_arguments(cli);
        free(cli);
        cli = NULL;
    }
    //cli->start = false;
    exit(0);
}

int main(void) {
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal setup failed");
    }
    if (1 != exit_flag) {
        signal(SIGTERM, clean_resources);
    }
    cli = cli_initialize();
    cli_start(cli, container_commands);
    return 0;
}