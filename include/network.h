#ifndef NETWORK_H
#define NETWORK_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <linux/if_link.h>
#include <pthread.h>

#include "linkedlist.h"

#define BRIDGE "br0"
#define MAX_THREADS 8

typedef struct network_interface_t {
    char ip[INET_ADDRSTRLEN];
    char subnet[INET_ADDRSTRLEN];
    int veth_number;
} network_interface_t;

typedef struct network_ip_veth_t {
    char host[INET6_ADDRSTRLEN];
    char ip[INET6_ADDRSTRLEN];
    int veth_number;
    int subnet;
} network_ip_veth_t;

// this is 
typedef struct thread_data_t{
    network_interface_t *network_interface;
    linkedlist_t *linkedlist;
    int veth_number;
    unsigned int start_ip;
    unsigned int end_ip;
} thread_data_t;

void network_setup_host(network_ip_veth_t * network_ip_veth, pid_t pid, char * container_name, network_ip_veth_t * network_bridge);

network_ip_veth_t * network_create_bridge(linkedlist_t * linkedlist);

network_interface_t * network_get_host_information(void);

int network_remove_veth(network_ip_veth_t * network_ip_veth);

linkedlist_t * network_get_free_ips(network_interface_t * network_interface);

#endif