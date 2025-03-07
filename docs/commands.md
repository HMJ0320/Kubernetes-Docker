Summary of Commands:

(in Bash)

1. Create network namespace:
sudo ip netns add mycontainer

2. Create a veth pair:
sudo ip link add veth0 type veth peer name veth1

3. Assign IP to host's veth0 interface:
sudo ip addr add 10.0.2.15/24 dev veth0
sudo ip link set veth0 up

4. Move veth1 to the container's network namespace:
sudo ip link set veth1 netns mycontainer

5. Assign IP to container's veth1 interface:
sudo ip netns exec mycontainer ip addr add 10.0.2.16/24 dev veth1
sudo ip netns exec mycontainer ip link set veth1 up

6. Bring up the loopback interface inside the container:
sudo ip netns exec mycontainer ip link set lo up

7. Enable IP forwarding on the host (optional for routing):
sudo sysctl -w net.ipv4.ip_forward=1

8. Add a default route inside the container (optional):
sudo ip netns exec mycontainer ip route add default via 10.0.2.15

9. Ping the host from the container:
sudo ip netns exec mycontainer ping -c 4 10.0.2.15

10. Ping the container from the host:
ping -c 4 10.0.2.16


HOST:
sudo ip link add veth0 type veth peer name veth1

sudo ip addr add 10.0.2.15/24 dev veth0

sudo ip link set veth0 up

sudo ip link set veth1 netns <pid of child space> # see if there is another way of adding veth1 on the container instead

sudo sysctl -w net.ipv4.ip_forward=1





CONTAINER:
sudo ip addr add 10.0.2.16/24 dev veth1

sudo ip link set veth1 up

sudo ip link set lo up

sudo ip route add default via 10.0.2.15





NOTES:
should be able to ping now

