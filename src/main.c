#include <stdio.h>
#include <signal.h>

#include "../include/container.h"
#include "../include/cli.h"
#include "../include/hashtable.h"

cli_t * cli = NULL;
volatile sig_atomic_t sigint_received = 0;

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

void sigint_handler(int sig) {
    // If the input buffer is allocated, free it before exiting
    if (cli->command != NULL) {
        free(cli->command);
        cli->command = NULL;
        printf("\nInput buffer freed. Program exiting due to SIGINT...\n");
    }
}

int main(void) {
    network_interface_t * network_interface = network_get_host_information();
    
    linkedlist_t * linkedlist = network_get_free_ips(network_interface); //multithread later to make pings more optimal

    //node_t * node = linkedlist->head->next;
    //for (size_t i = 0; i < linkedlist->count; i++) {
    //    network_ip_veth_t * niv = (network_ip_veth_t *)node->data;
    //    printf("ip: %s, veth: %d\n", niv->ip, niv->veth_number);
    //    node = node->next;
    //}

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal setup failed");
    }
    if (1 != sigint_received) {
        signal(SIGTERM, clean_resources);
    }
    //signal(SIGTSTP, clean_resources); using ctrl z and pauses application
    cli = cli_initialize();
    cli->data3 = linkedlist;
    cli_start(cli, container_commands);
    return 0;
}

