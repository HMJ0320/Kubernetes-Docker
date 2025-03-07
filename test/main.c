#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mount.h>
#include <string.h>
#include <fcntl.h>

#define STACK_SIZE (1024 * 1024)  // 1 MB stack size for the new process

// Function that will be executed in the new namespace
int child_func(void *arg) {
    printf("child process: %d\n", getpid());
    // Set up a new filesystem namespace (mount a new root)
    if (chroot("/") == -1) {
        perror("chroot failed");
        exit(1);
    }

    // Change working directory to the new root
    if (chdir("/") == -1) {
        perror("chdir failed");
        exit(1);
    }

    // Mount /proc (to simulate process namespace isolation)
    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        perror("mount /proc failed");
        exit(1);
    }

    // Start a new shell (bash or sh, depending on availability)
    execlp("/bin/bash", "/bin/bash", (char *)NULL);
    // If exec fails
    perror("execlp failed");


    return 1;
}

int main() {
    // Allocate memory for the child process's stack
    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("Failed to allocate stack");
        return 1;
    }

    // Clone the process and create a new namespace
    pid_t pid = clone(child_func, stack + STACK_SIZE, CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWNET | SIGCHLD, NULL);
    
    if (pid == -1) {
        perror("clone failed");
        free(stack);
        return 1;
    }

    // Parent waits for the child process to finish
    waitpid(pid, NULL, 0);

    free(stack);
    return 0;
}
