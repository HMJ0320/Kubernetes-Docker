/** @file cli.h
 *
 * @brief CLI file that defines how to get the user input.
 */

#ifndef CLI_H
#define CLI_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

// Maximum size of the number of characters the user can enter.
//
#define MAX_SIZE 64

// Maximum size of the number of characters the user can enter for stdin path.
//
#define MAX_PATH_SIZE 256

#define MAX_ARGS 8

extern volatile sig_atomic_t exit_flag;

// CLI structure that holds the command injected from the user and a valid
// session.
//
typedef struct cli_t
{
    bool   start;
    char * command;
    void * data1;
    void * data2;
    void * data3;
    int * argc;
    char ** argv;
} cli_t;

// Initializes the CLI struct.
//
cli_t * cli_initialize(void); // initialize the CLI structure

// Stores data from arguments when starting program.
//
int cli_arguments(cli_t * cli, int argc, char * argv[]);

// Frees any arguments when starting the program.
//
void cli_free_arguments(cli_t * cli);

// Gets the user standard input to inject commands and text into the program.
//
char * cli_input(void); // reads the user input from cli

// Starts the cli however, may need to customize function for needs of program.
//
void cli_start(cli_t * cli, void (*function)(char *, cli_t *)); // start the CLI

#endif /* cli.h */

/*** end of file ***/