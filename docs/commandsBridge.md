HOST:
sudo ip link add veth0 type veth peer name veth1

sudo ip addr add 10.0.2.15/24 dev veth0

sudo ip link set veth0 up

sudo ip link set veth1 netns <pid of child space> # see if there is another way of adding veth1 on the container instead

sudo sysctl -w net.ipv4.ip_forward=1



sudo ip netns attach <name> <namespace pid>
sudo ip netns exec <name>

CONTAINER:
sudo ip addr add 10.0.2.16/24 dev veth1

sudo ip link set veth1 up

sudo ip link set lo up

sudo ip route add default via 10.0.2.15

NOTES:
should be able to ping now which only works for one container







# Bash Script
#!/bin/bash
# Script to create network namespaces using a bridge network with three containers

# Ensure script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

# Clean up any existing configuration
echo "Cleaning up existing configuration..."
ip netns del container1 2>/dev/null
ip netns del container2 2>/dev/null
ip netns del container3 2>/dev/null
ip link del veth1 2>/dev/null
ip link del veth2 2>/dev/null
ip link del veth3 2>/dev/null
ip link del br0 2>/dev/null








# Below is what matters and need to create a bridge with one ip and use to connect all containers
# Create a bridge
echo "Creating bridge interface..."
ip link add name br0 type bridge
ip link set br0 up
ip addr add 10.0.2.100/24 dev br0

# Create network namespaces
echo "Creating network namespaces..."
ip netns add container1
ip netns add container2
ip netns add container3

# Create veth pairs
echo "Creating veth pairs..."
ip link add veth1 type veth peer name c1-eth0
ip link add veth2 type veth peer name c2-eth0
ip link add veth3 type veth peer name c3-eth0

# Move interfaces to namespaces
echo "Moving interfaces to namespaces..."
ip link set c1-eth0 netns container1
ip link set c2-eth0 netns container2
ip link set c3-eth0 netns container3

# Connect veth pairs to bridge
echo "Connecting veth pairs to bridge..."
ip link set veth1 up
ip link set veth2 up
ip link set veth3 up
ip link set veth1 master br0
ip link set veth2 master br0
ip link set veth3 master br0

# Configure container1
echo "Configuring container1..."
ip netns exec container1 ip link set lo up
ip netns exec container1 ip link set c1-eth0 up
ip netns exec container1 ip addr add 10.0.2.4/24 dev c1-eth0
ip netns exec container1 ip route add default via 10.0.2.100

# Configure container2
echo "Configuring container2..."
ip netns exec container2 ip link set lo up
ip netns exec container2 ip link set c2-eth0 up
ip netns exec container2 ip addr add 10.0.2.5/24 dev c2-eth0
ip netns exec container2 ip route add default via 10.0.2.100

# Configure container3
echo "Configuring container3..."
ip netns exec container3 ip link set lo up
ip netns exec container3 ip link set c3-eth0 up
ip netns exec container3 ip addr add 10.0.2.6/24 dev c3-eth0
ip netns exec container3 ip route add default via 10.0.2.100

# Enable IP forwarding
echo "Enabling IP forwarding..."
echo 1 > /proc/sys/net/ipv4/ip_forward

# Set up routing between bridge and host interface
echo "Setting up routing between bridge and host interface..."


# Should not need below and was working without setting postrouting
# Create routing rules for traffic between bridge and host network
iptables -t nat -A POSTROUTING -s 10.0.2.0/24 -o enp0s3 -j MASQUERADE
iptables -A FORWARD -i enp0s3 -o br0 -j ACCEPT
iptables -A FORWARD -i br0 -o enp0s3 -j ACCEPT
iptables -A FORWARD -i br0 -o br0 -j ACCEPT

# Testing connectivity
echo "Testing from container1 to host..."
ip netns exec container1 ping -c 2 10.0.2.15

echo "Testing from container2 to host..."
ip netns exec container2 ping -c 2 10.0.2.15

echo "Testing from container3 to host..."
ip netns exec container3 ping -c 2 10.0.2.15

echo "Testing container-to-container connectivity..."
echo "Container1 to Container2:"
ip netns exec container1 ping -c 2 10.0.2.5
echo "Container1 to Container3:"
ip netns exec container1 ping -c 2 10.0.2.6
echo "Container2 to Container3:"
ip netns exec container2 ping -c 2 10.0.2.6

echo "Setup complete!"