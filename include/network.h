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

#include "linkedlist.h"

typedef struct network_interface_t {
    char ip[INET_ADDRSTRLEN];
    char subnet[INET_ADDRSTRLEN];
    int veth_number;
} network_interface_t;

typedef struct network_ip_veth_t {
    char ip[INET6_ADDRSTRLEN];
    int veth_number;
} network_ip_veth_t;

network_interface_t * network_get_host_information(void);

linkedlist_t * network_get_free_ips(network_interface_t * network_interface);

#endif