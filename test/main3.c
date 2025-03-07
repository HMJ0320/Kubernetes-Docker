#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "../include/linkedlist.h"

int get_network_interface_information() {
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;
    char ip[INET_ADDRSTRLEN];
    char subnet[INET_ADDRSTRLEN];

    // Get the list of all network interfaces
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 1;
    }

    // Loop through each interface
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        // If it's an IPv4 address (AF_INET)
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // Get IP address
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
            
            // Get Subnet Mask (using ifa_netmask)
            sa = (struct sockaddr_in *) ifa->ifa_netmask;
            inet_ntop(AF_INET, &sa->sin_addr, subnet, sizeof(subnet));

            // Print the information
            printf("Interface: %s\n", ifa->ifa_name);
            printf("IP Address: %s\n", ip);
            printf("Subnet Mask: %s\n\n", subnet);
        }
    }

    // Free the memory allocated by getifaddrs()
    freeifaddrs(ifaddr);

    return 0;
}

int main(void) {
    linkedlist_t * linkedlist = linkedlist_init();
    for (int i = 0; i < 10; i++) {
        int * data = (int *)malloc(sizeof(int));
        *data = i;
        linkedlist_append(linkedlist, data);
    }

    node_t * node = linkedlist->head->next;
    for (size_t i = 0; i < linkedlist->count; i++) {
        printf("%d\n", *(int *)node->data);
        node = node->next;
    }
    
    int * data1 = linkedlist_pop(linkedlist);
    printf("\n%d\n", *data1);
    free(data1);

    int * data2 = linkedlist_popleft(linkedlist);
    printf("%d\n\n", *data2);
    free(data2);

    node = linkedlist->head->next;
    for (size_t i = 0; i < linkedlist->count; i++) {
        printf("%d\n", *(int *)node->data);
        node = node->next;
    }

    linkedlist_free_data(linkedlist);
    linkedlist_free(linkedlist);
    return 0;
}


























#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CONTAINER_NETNS_PATH "/var/run/netns/container_netns"

// Function to execute system commands
void execute_system_command(const char *cmd) {
    if (system(cmd) == -1) {
        perror("System command failed");
        exit(1);
    }
}

// Create veth pair and configure the network interfaces
void create_veth_pair(const char *veth1, const char *veth2) {
    char cmd[256];

    // Create the veth pair using system command
    snprintf(cmd, sizeof(cmd), "ip link add %s type veth peer name %s", veth1, veth2);
    execute_system_command(cmd);

    // Bring up the host-side veth interface
    snprintf(cmd, sizeof(cmd), "ip link set %s up", veth1);
    execute_system_command(cmd);
}

// Move veth interface into the container's namespace
void move_veth_to_container(pid_t pid, const char *veth2) {
    char cmd[256];

    // Move the veth interface into the container's namespace using system command
    snprintf(cmd, sizeof(cmd), "ip link set %s netns %d", veth2, pid);
    execute_system_command(cmd);
}

// Set IP address and default gateway inside the container
void configure_container_network(pid_t pid, const char *veth2) {
    char cmd[256];

    // Configure the container's veth interface inside its namespace
    snprintf(cmd, sizeof(cmd), "ip netns exec %d ip addr add 192.168.1.2/24 dev %s", pid, veth2);
    execute_system_command(cmd);

    // Bring the veth interface up inside the container
    snprintf(cmd, sizeof(cmd), "ip netns exec %d ip link set %s up", pid, veth2);
    execute_system_command(cmd);

    // Set up default route (gateway) inside the container
    snprintf(cmd, sizeof(cmd), "ip netns exec %d ip route add default via 192.168.1.1", pid);
    execute_system_command(cmd);
}

// Set up container's network namespace using system commands (via `unshare` + `clone`)
void create_network_namespace_and_run_container(pid_t pid) {
    char cmd[256];

    // Create a new network namespace using unshare and then clone the process into it
    snprintf(cmd, sizeof(cmd), "unshare --net --pid --fork --mount-proc --pid-map=0:1:1 -- bash -c 'echo Container PID: $$; sleep 10'");
    execute_system_command(cmd);

    // Wait for the process to finish
    wait(NULL);
}

int main() {
    pid_t pid1, pid2;

    // Create two containers using system commands (via clone and unshare)
    create_network_namespace_and_run_container(pid1); // First container
    create_network_namespace_and_run_container(pid2); // Second container

    // Set up veth pair for communication between host and containers
    create_veth_pair("veth1", "veth2");
    create_veth_pair("veth3", "veth4");

    // Move veth interfaces into respective containers
    move_veth_to_container(pid1, "veth2");
    move_veth_to_container(pid2, "veth4");

    // Configure the host's IP addresses
    execute_system_command("ip addr add 192.168.1.1/24 dev veth1");
    execute_system_command("ip link set veth1 up");

    execute_system_command("ip addr add 192.168.2.1/24 dev veth3");
    execute_system_command("ip link set veth3 up");

    // Configure container's network interfaces
    configure_container_network(pid1, "veth2");
    configure_container_network(pid2, "veth4");

    printf("Host network and containers are set up.\n");

    return 0;
}
