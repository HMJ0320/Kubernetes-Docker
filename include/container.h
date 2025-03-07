#ifndef CONTAINER_H
#define CONTAINER_H

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

#include "cli.h"
#include "hashtable.h"
#include "linkedlist.h"
#include "network.h"

#define MEGABYTE (1024 * 1024)
#define MAXSIZE 64

typedef struct container_t {
    char container_name[MAX_SIZE];
    int parent_pid;
    int child_pid;
    char * stack_front;
    char * stack_end;
    size_t container_size;
} container_t;

int child_func(void * arg);

char * container_stack(container_t * container);

int container_init(cli_t * cli);

int container_delete(hashtable_t * containers, hashtable_t * container_networks);

void container_cleanup(cli_t * cli);

void container_commands(char * command, cli_t * cli);

#endif 