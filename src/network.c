#include "../include/network.h"

// creates the network namespace and needs to be used in child_func
void network_create_namespace() {
    if (unshare(CLONE_NEWNET) == 1) {
        perror("Failed to create network namespace.\n");
        return -1;
    }
}

//
void network_configure() {

}

void network_configure_host_ip(network_ip_veth_t * network_ip_veth) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ip addr add %s/%d dev veth%d", network_ip_veth->ip, /subnet, network_ip_veth->veth_number - 1);
    system(cmd);
}

void network_configure_host_veth(network_ip_veth_t * network_ip_veth) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ip link set veth%d up", network_ip_veth->veth_number - 1);
    system(cmd);
}

// assigns the virtual ethernet but more than likely should create veth in pairs with the ip
void network_create_virtual_ethernet(network_interface_t * network_interface, network_ip_veth_t * network_ip_veth_t) {

}

// create a function that recieves the host network information
network_interface_t * network_get_host_information(void) {
    // create a pointer of pointer network interface structs later so that we can store all of them
    network_interface_t * network_interface_host = (network_interface_t *)malloc(sizeof(network_interface_t));

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
            
            if (0 != strcmp("lo", ifa->ifa_name)) {
                memcpy(network_interface_host->ip, ip, INET_ADDRSTRLEN);
                memcpy(network_interface_host->subnet, subnet, INET_ADDRSTRLEN);
                break;
            }

            //// Print the information
            //printf("Interface: %s\n", ifa->ifa_name);
            //printf("IP Address: %s\n", ip);
            //printf("Subnet Mask: %s\n\n", subnet);
        }
    }

    // Free the memory allocated by getifaddrs()
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

linkedlist_t * network_get_free_ips(network_interface_t * network_interface) {
    linkedlist_t * linkedlist = linkedlist_init();
    struct in_addr base_ip;
    struct in_addr subnet_mask;
    struct in_addr test_ip;

    printf("ip: %s, subnet: %s\n", network_interface->ip, network_interface->subnet);
    inet_pton(AF_INET, network_interface->ip, &base_ip);
    inet_pton(AF_INET, network_interface->subnet, &subnet_mask);

    base_ip.s_addr = base_ip.s_addr & subnet_mask.s_addr;

    unsigned int max_ip = (~ntohl(subnet_mask.s_addr) & 0x00FFFFFF);
    max_ip = 5;
    printf("max ip: %d\n", max_ip);
    //multithread this so that it can ping multiple ips at once
    for (unsigned int i = 1; i < max_ip; i++) {  // Exclude network (0) and broadcast (255)
        network_get_next_ip(&base_ip, i, &test_ip);
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &test_ip, ip_str, INET_ADDRSTRLEN);
        printf("Checking IP: %s\n", ip_str);

        if (!network_ip_in_use(ip_str)) {
            network_ip_veth_t * network_ip_veth = (network_ip_veth_t *)malloc(sizeof(network_ip_veth_t));
            memcpy(network_ip_veth->ip, ip_str, INET_ADDRSTRLEN);
            network_ip_veth->veth_number = i;
            linkedlist_append(linkedlist, network_ip_veth);
            printf("Found free IP: %s\n", ip_str);
        }
    }
    return linkedlist;
}

// create a function that goes through the ips in the subnet until one is free if not print out no additional containers can be created

// once we do this, we can keep track of a hashmap that keeps open virtual eth numbers in place and 

// create pairs of ip and veth, if a container is deleted we add it to the end of the linked list, if a container is created we remove it from the front, meaning that containers structs need to store the ip and veth as well.

// need to create a ssh server inside of the network namespace inside of the container

// potentially need to create a routing table for the container as well.

// create some dynamic way to ensure that containers cant keep taking unlimited memory because if fail to allocate, then print out that the computer does not have additional space.

// need to create a mounted directory that the container can use as its root directory and doesnt have access to the host computer folders

// potentially set up authentication on the container.

// may want to split up code between network as just ip, veth, ssh, routing,
// directory creation can be done on create_container function

// lastly test that everything works.