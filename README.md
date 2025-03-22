# Self created Container Creation & Management Tool
## Description
- Ths project is an independent project in order to replicate container creation such as Docker and container managment such as Kubernetes
- The project was created mainly to learn more about namespaces and cgroups and how linux creates and manages proesses which can be inclusive of containers
## Installation Instructions
- This project requires the following
  1. Linux Ubuntu
  2. C latest version
  3. Make
- Instructions to install
  1. Pull the project from Git
  2. Ensure the bin folder exists in the root directory of the project
  3. run command 'make' to link libraries and create binary
  4. cd bin
  5. run application with 'sudo ./container'
  6. The application runs in the cli and runs as a cli tool
## Usage
- Install application as above
- run application with sudo privelidges
- There are 4 main commands:
  1. create_container - creates a container and returns a pid for the container
  2. delete_container - deletes a container and kills the process for the container and returns the resources to parent process
  3. list_containers - lists containers that have been created
  4. help - lists all available commands
 - Run any of the commands above and enter prompted information
 - ping container IPs from host device can check current network configuration using 'ip address'
## Features
1. Container creation
2. Container deletion
3. List Containers
4. Assign IP to container so host device can reach
5. Assign IP to bridge on host to connect all containers on host device
## Implementation
- Created independent libraries using standard C libraries:
  1. Hashtables
  2. LinkedList
  3. CLI
- Need to add more
## Notes
- Application needs sudo permissions because cloning the root process requires admin access
- Potentially explore Podman implementation of Containers as it does not require root access
- Add in server capabilities so that container can be run remotely via client-server model and own network protocol creation
