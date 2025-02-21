/** @file cli.c
 *
 * @brief CLI file that defines how to get the user input.
 */

#include "../include/cli.h"

/*!
 * @brief Initializes the CLI struct.
 * @return A pointer to the CLI struct initialized.
 */
cli_t * cli_initialize(void) {
    // Allocate memory for the cli structure and handle allocation failure.
    cli_t * cli = (cli_t *)malloc(sizeof(cli_t));
    if (NULL == cli) {
        printf("Failed to allocate memory for cli\n");
        goto exit;
    }
    // Set the session to false.
    cli->start = false;

exit:
    return cli;
} /* initialize_cli() */

/*!
 * @brief Stores data from arguments when starting program.
 * @param[in] cli A pointer to the cli in order to end the current session.
 * @return Returns whether the function was successful or not.
 */
int cli_arguments(cli_t * cli, int argc, char * argv[]) {
    // Creates a pointer for argc and stores into cli object.
    int   success  = 0;
    int * argc_ptr = (int *)malloc(sizeof(int));
    if (NULL == argc_ptr) {
        success = 1;
        printf("Failed to allocate memory for argv.\n");
        goto exit;
    }
    *argc_ptr = argc;
    cli->argc = argc_ptr;

    // Creates a string array to store argv into the cli object.
    char ** argv_ptr = malloc(sizeof(char *) * argc);
    if (NULL == argv_ptr) {
        free(argc_ptr);
        cli->argc = NULL;
        success   = 1;
        printf("Failed to allocate memory for argv.\n");
        goto exit;
    }

    // Iterates over all arguments and stores data.
    for (int i = 0; i < argc; i++) {
        char * argv_index = (char *)malloc(MAX_PATH_SIZE);
        argv_ptr[i]       = argv_index;
        strncpy(argv_index, argv[i], MAX_PATH_SIZE - 1);
        argv_index[MAX_PATH_SIZE - 1] = '\0';
        if (!argv_ptr[i]) {
            for (int j = 0; j < i; j++) {
                free(argv_ptr[j]);
                argv_ptr[j] = NULL;
            }
            free(argv_ptr);
            argv_ptr = NULL;
            success  = 1;
            goto exit;
        }
    }
    cli->argv = argv_ptr;

exit:
    return success;
} /* cli_arguments() */

/*!
 * @brief Frees any arguments when starting the program.
 * @param[in] cli A pointer to the cli in order to end the current session.
 */
void cli_free_arguments(cli_t * cli) {
    // Iterates over cli object cli data and frees allocations.
    if (cli->argc != NULL && cli->argv != NULL) {
        int index = *cli->argc;
        for (int i = 0; i < index; i++) {
            if (cli->argv[i] != NULL) {
                free(cli->argv[i]);
                cli->argv[i] = NULL;
            }
        }
        free(cli->argv);
        cli->argv = NULL;
        free(cli->argc);
        cli->argc = NULL;
    }
} /* cli_free_arguments() */

/*!
 * @brief Gets the user standard input to inject commands and text into the
 * program.
 * @return A string to the text the user injected limited to 64 characters
 * including the string nullifier.
 */
char * cli_input(void) {
    // Create a pointer in which getline will allocate stdin for.
    char * command = NULL;
    // Create variables to store data necessary for getline function.
    size_t size = 0;
    int    read;
    // Stores input from standard input into command.
    read = getline(&command, &size, stdin); // this is problematic when sending signal handlers as it doesnt free the memory pointed by command. Although this is probably not a risk, it is still a mem leak.
    // Handles failure to getline.
    if (read == -1) {
        printf("getline failed\n");
        free(command);
        command = NULL;
        goto exit;
    }
    // Validates that the user input is 64 bytes long.
    while (read > MAX_SIZE) {
        printf("User input was too long, please keep within 64 characters\n");
        printf("Re-enter user input: ");
        read = getline(&command, &size, stdin);
        // Handles failure to getline.
        if (read == -1) {
            printf("getline failed\n");
            free(command);
            command = NULL;
            goto exit;
        }
    }
    // Sets the newline of the input into a null terminator and ensures 64 bytes including the terminator.
    if (read > 0 && command[read - 1] == '\n') {
        command[read - 1] = '\0';
    }
exit:
    return command;
} /* cli_input() */

/*!
 * @brief Ends the current session of the cli.
 * @param[in] cli A pointer to the cli in order to end the current session.
 */
static void cli_end(cli_t * cli) {
    // If the command is not empty and is equal to 'exit'.
    if ((NULL != cli->command) && (0 == strcmp(cli->command, "exit"))) {
        // Sets the cli session to false to exit the loop.
        //
        printf("Exiting CLI\n");
        cli->start = false;
    }
} /* cli_end() */

/*!
 * @brief Starts the cli however, may need to customize function for needs of
 * program.
 * @param[in] A pointer to the cli in order to start the session.
 */
void cli_start(cli_t * cli, void (*function)(char *, cli_t *)) {
    // Sets the session to true to ensure a loop to inject commands.
    cli->start = true;
    cli->data1 = NULL;
    cli->data2 = NULL;
    cli->data3 = NULL;
    // Loops as long as the session is set to true to inject commands.
    while (cli->start) {
        if (NULL == cli->data1) {
            function("", cli);
        }
        printf("Enter Command: ");
        // Gets the input from the standard input.
        cli->command = cli_input();
        // Checks if the command was 'exit' to end the session.
        cli_end(cli);
        if (function != NULL) {
            function(cli->command, cli);
        }
        // Free the allocated memory for the user input for new input.
        free(cli->command);
        cli->command = NULL;
    }
    if (cli->argc != NULL) {
        cli_free_arguments(cli); // Call this when cli_start finishes
    }
    // Free the memory for the cli
    free(cli);
    cli = NULL;
} /* cli_start() */

/*** end of file ***/