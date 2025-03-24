#include "../include/container.h"

//static int container_mount(void) {
//    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
//        perror("Failed to mount proc");
//        return 1;
//    }
//    if (mount("sysfs", "/sys", "sysfs", 0, NULL) == -1) {
//        perror("Failed to mount sysfs");
//        return 1;
//    }
//    if (mount("devtmpfs", "/dev", "devtmpfs", MS_MGC_VAL, NULL) == -1) {
//        perror("Failed to mount devtmpfs");
//        return 1;
//    }
//    return 0;
//}

int child_func(void * arg) {
    if (NULL == arg) {
        printf("Arguments have been passed to child process.\n");
    }

    //if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
    //    perror("mount /proc failed");
    //    exit(1);
    //}

    if (setsid() == -1) {
        perror("setsid failed");
        exit(1);
    }

    while (1) {
        printf("Container PID %d running independently...\n", getpid());
        sleep(100);
    }

    return 0;
}

char * container_stack(container_t * container) {
    printf("Enter container size in Mb: ");
    char * stack_size_char = cli_input();
    size_t stack_size = atoi(stack_size_char);
    // meed to do some input check here to make sure that the stack size was indeed a number
    memccpy(&container->container_size, &stack_size, 1, sizeof(size_t));
    printf("Container size: %ld bytes\n", MEGABYTE*stack_size);
    char * child_stack = (char *)malloc(stack_size*MEGABYTE);
    container->stack_front = child_stack;
    if (NULL == child_stack) {
        perror("Failed to allocate for container stack.\n");
        free(stack_size_char);
        stack_size_char = NULL;
        return NULL;
    }
    char * stack_top = child_stack + (stack_size*MEGABYTE);
    container->stack_end = stack_top;
    free(stack_size_char);
    stack_size_char = NULL;
    return stack_top;
}

static pid_t container_clone(container_t * container, cli_t * cli, char * container_name) {
    char * stack_top = container_stack(container);
    if (NULL == stack_top) {
        perror("Failed to allocate stack.\n");
        return -1;
    }
    network_ip_veth_t * network_ip_veth = linkedlist_popleft(cli->data3);
    printf("Found free Host: %s, IP: %s, VETH: %d, Subnet: %d\n", network_ip_veth->host, network_ip_veth->ip, network_ip_veth->veth_number - 1, network_ip_veth->subnet);
    network_ip_veth_t * network_bridge = hashtable_search(cli->data2, "br", 's');
    pid_t pid = clone(child_func, stack_top, SIGCHLD | CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWNET, network_ip_veth); // add this for pid to be one in container CLONE_NEWPID
    network_setup_host(network_ip_veth, pid, container_name, network_bridge);
    hashtable_insert(cli->data2, container_name, network_ip_veth, 's');
    memccpy(&container->child_pid, &pid, 1, sizeof(pid_t));
    if (-1 == pid) {
        perror("Failed to clone.\n");
        free(container);
        container = NULL;
        return -1;
    }
    return pid;
}

int container_init(cli_t * cli) {
    hashtable_t * hashtable = (hashtable_t *)cli->data1;
    container_t * container = (container_t *)malloc(sizeof(container_t));
    printf("Enter container name: ");
    char * container_name = cli_input();
    pid_t pid_temp = container_clone(container, cli, container_name);
    printf("parent pid: %d\n", getpid()); // potentially get rid of
    printf("child pid: %d\n", pid_temp); // potentially get rid of
    memccpy(&container->child_pid, &pid_temp, 1, sizeof(pid_t));
    hashtable_insert(hashtable, container_name, container, 's');
    free(container_name);
    container_name = NULL;
    return 0;
}

// need to add the cli and make it so that we append the network interface onto the end of the linkedlist of free addresses
int container_delete(hashtable_t * containers, hashtable_t * container_networks) {
    printf("Enter container name to delete: ");
    char * container_name = cli_input();
    container_t * container = hashtable_search(containers, container_name, 's');
    network_ip_veth_t * container_network = hashtable_search(container_networks, container_name, 's');
    if (NULL == container) {
        printf("Container does not exist.\n");
    } else {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo ip netns delete %s", container_name);
        if (system(cmd) != 0) {
            perror("Failed to remove netns name\n");
            return 0;
        }
        network_remove_veth(container_network);
        int killed = kill(container->child_pid, SIGKILL); // Try using SIGTERM or SIGKILL
        if (-1 == killed) {
            printf("Container '%s' could not be killed.\n", container_name);
        } else {
            printf("Container '%s' was killed.\n", container_name);
            int status;
            pid_t wpid = waitpid(container->child_pid, &status, 0);  // Wait for child to finish
            if (-1 == wpid) {
                perror("waitpid failed");
            } else {
                printf("Child process with PID %d has terminated.\n", container->child_pid);
            }
        }
        if (container) {
            if (container->stack_front) {
                free(container->stack_front);
                container->stack_front = NULL;
            }
            free(container);
            container = NULL;
        }
        hashtable_remove(containers, container_name, 's');
        hashtable_remove(container_networks, container_name, 's');
        if (container_network) {
            free(container_network);
            container_network = NULL;
        }
    }
    free(container_name);
    container_name = NULL;
    return 0;
}

void container_list_all(hashtable_t * hashtable) {
    printf("Containers:\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        item_t * item = hashtable->table[i];
        while (item) {
            printf("container name: %s\n", (char *)item->key);
            item = item->next;
        }
    }
}

void container_free_containers(cli_t * cli) {
    hashtable_t * hashtable = cli->data1;
    for (int i = 0; i < TABLE_SIZE; i++) {
        item_t * item = hashtable->table[i];
        while (item) {
            container_t * container = item->value;


            char * container_name = item->key;
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "sudo ip netns delete %s", container_name);
            if (system(cmd) != 0) {
                perror("Failed to remove netns name\n");
                return;
            }


            free(container->stack_front);
            container->stack_front = NULL;
            free(container);
            item = item->next;
        }
    }
    hashtable_free(cli->data1);
    cli->data1 = NULL;
}

void container_free_bridges_and_veths(cli_t * cli) {
    hashtable_t * bridges_and_ips = cli->data2;
    for (int i = 0; i < TABLE_SIZE; i++) {
        item_t * item = bridges_and_ips->table[i];
        while (item) {
            network_ip_veth_t * network_ip_veth = item->value;
            network_remove_veth(network_ip_veth);
            item = item->next;
        }
    }
    if (bridges_and_ips != NULL) {
        hashtable_free_values(cli->data2);
        hashtable_free(cli->data2);
    }
    cli->data2 = NULL;
}

void container_free_ips(cli_t * cli) {
    linkedlist_t * linkedlist = cli->data3;
    if (linkedlist != NULL) {
        linkedlist_free_data(cli->data3);
        linkedlist_free(cli->data3);
    }
    cli->data3 = NULL; 
}

void container_cleanup(cli_t * cli) {
    network_ip_veth_t * network_bridge = hashtable_search(cli->data2, "br", 's');
    if (NULL != network_bridge) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo ip link delete br%d", network_bridge->veth_number);
        if (system(cmd) != 0) {
            perror("Failed to remove bridge\n");
        }
    
        hashtable_remove(cli->data2, "br", 's');
        free(network_bridge);
        network_bridge = NULL;
    }

    container_free_containers(cli);
    container_free_bridges_and_veths(cli);
    container_free_ips(cli);
}

void container_list_commands(void) {
    printf("help - lists all commands available\n");
    printf("create_container - creates a new container of size n mb\n");
    printf("delete_container - deletes a container if it is has been created\n");
    printf("list_containers - lists all current containers that have been created during current session\n");
}

void container_commands(char * command, cli_t * cli) {
    if (NULL == cli->data1) {
        hashtable_t * containers = hashtable_init();
        cli->data1 = containers;
    }
    if (NULL == cli->data2) {
        hashtable_t * network = hashtable_init();
        cli->data2 = network;
    }
    if (NULL == cli->data3) {
        network_interface_t * network_interface = network_get_host_information();
        linkedlist_t * ips = network_get_free_ips(network_interface); 
        free(network_interface);
        cli->data3 = ips;
    }
    if (NULL == hashtable_search(cli->data2, "br", 's')) {
        network_ip_veth_t * network_bridge = network_create_bridge(cli->data3);
        hashtable_insert(cli->data2, "br", network_bridge, 's');
    }
    
    // Commands handling
    if (strcmp(command, "create_container") == 0) {
        container_init(cli);
    } else if (strcmp(command, "delete_container") == 0) {
        container_delete(cli->data1, cli->data2);
    } else if (strcmp(command, "list_containers") == 0) {
        container_list_all(cli->data1);
    } else if (strcmp(command, "help") == 0) {
        container_list_commands();
    } else if (strcmp(command, "exit") == 0) {
        container_cleanup(cli);
    }
}