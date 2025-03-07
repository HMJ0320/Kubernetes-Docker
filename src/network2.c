#include "../include/network.h"

// Function to enable IP forwarding on the host
void network_setup_host(network_ip_veth_t * network_ip_veth, pid_t pid, char * container_name, network_ip_veth_t * network_bridge) {
    char cmd[256];
    printf("cm sudo ip link add veth%d type veth peer name veth%d\n\n", network_ip_veth->veth_number - 1, network_ip_veth->veth_number);
    snprintf(cmd, sizeof(cmd), "sudo ip link add veth%d type veth peer name veth%d", network_ip_veth->veth_number - 1, network_ip_veth->veth_number);
    if (system(cmd) != 0) {
        perror("Failed to add veths.\n");
    }
    
    printf("cm sudo ip addr add %s/%d dev veth%d\n\n", network_ip_veth->host, network_ip_veth->subnet, network_ip_veth->veth_number - 1);
    snprintf(cmd, sizeof(cmd), "sudo ip addr add %s/%d dev veth%d", network_ip_veth->host, network_ip_veth->subnet, network_ip_veth->veth_number - 1);
    if (system(cmd) != 0) {
        perror("Failed to add ip address.\n");
    }

    printf("cm sudo ip link set veth%d up\n\n", network_ip_veth->veth_number - 1);
    snprintf(cmd, sizeof(cmd), "sudo ip link set veth%d up", network_ip_veth->veth_number - 1);
    if (system(cmd) != 0) {
        perror("Failed to set veth up.\n");
    }

    printf("cm sudo ip link set veth%d master br0\n\n", network_ip_veth->veth_number - 1);
    snprintf(cmd, sizeof(cmd), "sudo ip link set veth%d master br0", network_ip_veth->veth_number - 1);
    if (system(cmd) != 0) {
        perror("Failed to set master.\n");
    }

    printf("cm sudo ip link set veth%d netns %d\n\n", network_ip_veth->veth_number, pid);
    snprintf(cmd, sizeof(cmd), "sudo ip link set veth%d netns %d", network_ip_veth->veth_number, pid);
    if (system(cmd) != 0) {
        perror("Failed to move veth to container.\n");
    }

    printf("cm sudo sysctl -w net.ipv4.ip_forward=1\n\n");
    snprintf(cmd, sizeof(cmd), "sudo sysctl -w net.ipv4.ip_forward=1");
    if (system(cmd) != 0) {
        perror("Failed to add ip address.\n");
    }

    char * name = container_name;

    snprintf(cmd, sizeof(cmd), "sudo ip netns attach %s %d", name, pid);
    if (system(cmd) != 0) {
        perror("\n");
    }

    snprintf(cmd, sizeof(cmd), "sudo ip netns exec %s ip addr add %s/%d dev veth%d", name, network_ip_veth->ip, network_ip_veth->subnet, network_ip_veth->veth_number);
    if (system(cmd) != 0) {
        perror("\n");
    }

    snprintf(cmd, sizeof(cmd), "sudo ip netns exec %s ip link set veth%d up", name, network_ip_veth->veth_number);
    if (system(cmd) != 0) {
        perror("\n");
    }

    snprintf(cmd, sizeof(cmd), "sudo ip netns exec %s ip link set lo up", name);
    if (system(cmd) != 0) {
        perror("\n");
    }

    snprintf(cmd, sizeof(cmd), "sudo ip netns exec %s ip route add default via %s", name, network_bridge->ip); // needs to be bridge ip
    if (system(cmd) != 0) {
        perror("\n");
    }

    snprintf(cmd, sizeof(cmd), "sudo ip netns exec %s ping -c 2 %s", name, network_ip_veth->host);
    if (system(cmd) != 0) {
        perror("\n");
    }
}

network_ip_veth_t * network_create_bridge(linkedlist_t * linkedlist) {
    network_ip_veth_t * network_bridge  = linkedlist_pop(linkedlist);
    network_bridge->veth_number = 0;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo ip link add name br0 type bridge");
    if (system(cmd) != 0) {
        perror("Failed to create bridge.\n");
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "sudo ip link set br0 up");
    if (system(cmd) != 0) {
        perror("Failed to set bridge up.\n");
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "sudo ip addr add %s/%d dev br%d", network_bridge->ip, network_bridge->subnet, network_bridge->veth_number);
    if (system(cmd) != 0) {
        perror("Failed to add bridge ip.\n");
        return 0;
    }
    return network_bridge;
}

int network_remove_veth(network_ip_veth_t * network_ip_veth) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo ip link set veth%d down", network_ip_veth->veth_number - 1);
    if (system(cmd) != 0) {
        perror("Failed to set veth down.\n");
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "sudo ip link delete veth%d", network_ip_veth->veth_number - 1);
    if (system(cmd) != 0) {
        perror("Failed to delete veth \n");
        return 0;
    }
    return 1;
}


// Leave everything below alone

// Function to count the number of 1 bits in a 32-bit integer
int network_count_ones(uint32_t n) {
    int count = 0;
    while (n) {
        count += n & 1;  // Increment count if the least significant bit is 1
        n >>= 1;  // Right shift n by 1 bit
    }
    return count;
}

// Function to calculate the CIDR (network prefix) from the subnet mask
int network_get_cidr_from_subnet_mask(struct in_addr subnet_mask) { // make sure to name this network from the front to know what library its coming from
    return network_count_ones(subnet_mask.s_addr);  // Count the number of 1 bits in the subnet mask
}

network_interface_t * network_get_host_information(void) {
    network_interface_t *network_interface_host = (network_interface_t *)malloc(sizeof(network_interface_t));
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;
    char ip[INET_ADDRSTRLEN];
    char subnet[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
            sa = (struct sockaddr_in *)ifa->ifa_netmask;
            inet_ntop(AF_INET, &sa->sin_addr, subnet, sizeof(subnet));

            if (0 != strcmp("lo", ifa->ifa_name)) {
                memcpy(network_interface_host->ip, ip, INET_ADDRSTRLEN);
                memcpy(network_interface_host->subnet, subnet, INET_ADDRSTRLEN);
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    printf("IP Address: %s\n", network_interface_host->ip);
    printf("Subnet Mask: %s\n\n", network_interface_host->subnet);
    network_interface_host->veth_number = 0;
    return network_interface_host;
}

void network_get_next_ip(struct in_addr *base_ip, unsigned int increment, struct in_addr *result) {
    unsigned int ip_int = ntohl(base_ip->s_addr);
    ip_int += increment;
    result->s_addr = htonl(ip_int);
}

static int network_ip_in_use(const char *ip) {
    char command[128];
    snprintf(command, sizeof(command), "ping -c %d -W %d %s > /dev/null 2>&1", 3, 1, ip);
    return 0 == system(command);
}

// need to add multithreading here so that ping of network can happen faster
linkedlist_t * network_get_free_ips(network_interface_t *network_interface) {
    linkedlist_t * linkedlist = linkedlist_init();
    struct in_addr base_ip;
    struct in_addr subnet_mask;
    struct in_addr test_ip;

    printf("ip: %s, subnet: %s\n", network_interface->ip, network_interface->subnet);
    inet_pton(AF_INET, network_interface->ip, &base_ip);
    inet_pton(AF_INET, network_interface->subnet, &subnet_mask);

    base_ip.s_addr = base_ip.s_addr & subnet_mask.s_addr;

    int cidr = network_get_cidr_from_subnet_mask(subnet_mask);

    unsigned int max_ip = (~ntohl(subnet_mask.s_addr) & 0x00FFFFFF);
    unsigned int subnet = (~ntohl(subnet_mask.s_addr) & 0x00FFFFFF);
    //max_ip = 10; // Adjust if you want more IPs
    printf("max ip: %d\n", max_ip);
    int veth = 1;

    for (unsigned int i = 1; i < max_ip; i++) {
        network_get_next_ip(&base_ip, i, &test_ip);
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &test_ip, ip_str, INET_ADDRSTRLEN);

        if (!network_ip_in_use(ip_str)) {
            network_ip_veth_t *network_ip_veth = (network_ip_veth_t *)malloc(sizeof(network_ip_veth_t));
            memcpy(network_ip_veth->host, network_interface->ip, INET_ADDRSTRLEN);
            memcpy(network_ip_veth->ip, ip_str, INET_ADDRSTRLEN);
            network_ip_veth->subnet = cidr;
            network_ip_veth->veth_number = veth;
            linkedlist_append(linkedlist, network_ip_veth);
            veth += 2;

            printf("Found free Host: %s, IP: %s, VETH: %d, Subnet: %d\n", network_ip_veth->host, network_ip_veth->ip, network_ip_veth->veth_number, network_ip_veth->subnet);
        }
    }
    return linkedlist;
}

void* thread_function(void* arg) {
}

linkedlist_t * network_get_free_ips(network_interface_t *network_interface) {
    linkedlist_t *linkedlist = linkedlist_init();
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++) {
        
        pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    return linkedlist;
}
