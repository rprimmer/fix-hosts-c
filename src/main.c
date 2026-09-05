/**
 * @file main.c
 * @brief Command-line parsing and application dispatch.
 */

#include "actions.h"
#include "fix-hosts.h"
#include "system-actions.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Return the final pathname component used to invoke the program.
 *
 * @param path argv[0] from main().
 * @return Pointer into path at its basename.
 */
static const char *programName(const char *path) {
    const char *separator = strrchr(path, '/');
    return separator == NULL ? path : separator + 1;
}

/**
 * Select one action while rejecting conflicting command syntax.
 *
 * @param command Parsed command being populated.
 * @param action Newly requested action.
 * @return EXIT_SUCCESS when selected; EXIT_FAILURE if an action already exists.
 */
static int selectAction(Command *command, Action action) {
    if (command->action != ACTION_INVALID) {
        fputs("Error: actions cannot be combined.\n", stderr);
        return EXIT_FAILURE;
    }
    command->action = action;
    return EXIT_SUCCESS;
}

/**
 * Parse short options, long options, and positional actions.
 *
 * @param argc Argument count from main().
 * @param argv Argument vector from main().
 * @param command Output command populated from the arguments.
 * @return 0 when execution should continue, 1 after displaying help, or -1
 *         when the command line is invalid.
 */
static int parseArguments(int argc, char **argv, Command *command) {
    static const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"verbose", no_argument, NULL, 'v'},
        {"flush", no_argument, NULL, 'f'},
        {"add", required_argument, NULL, 'a'},
        {NULL, 0, NULL, 0}
    };

    opterr = 0;
    int option;
    while ((option = getopt_long(argc, argv, ":hvfa:", long_options, NULL)) != -1) {
        switch (option) {
        case 'h':
            usage(programName(argv[0]), stdout);
            return 1;
        case 'v':
            command->verbose = true;
            break;
        case 'f':
            if (selectAction(command, ACTION_FLUSH))
                return -1;
            break;
        case 'a':
            if (selectAction(command, ACTION_ADD))
                return -1;
            command->dns_name = optarg;
            break;
        case ':':
            fprintf(stderr, "Error: option -%c requires an argument.\n", optopt);
            return -1;
        default:
            fprintf(stderr, "Error: invalid option: -%c\n", optopt);
            return -1;
        }
    }

    int positional_count = argc - optind;
    if (positional_count > 1) {
        fprintf(stderr, "Error: expected one action, received %d arguments.\n",
                positional_count);
        return -1;
    }

    if (positional_count == 1) {
        Action positional_action = ACTION_INVALID;
        if (strcmp(argv[optind], "prep") == 0)
            positional_action = ACTION_PREP;
        else if (strcmp(argv[optind], "restore") == 0)
            positional_action = ACTION_RESTORE;
        else {
            fprintf(stderr, "Error: invalid action: %s\n", argv[optind]);
            return -1;
        }

        if (selectAction(command, positional_action))
            return -1;
    }

    if (command->action == ACTION_INVALID) {
        fputs("Error: no action provided.\n", stderr);
        return -1;
    }
    return 0;
}

/**
 * Parse the requested operation, load its paths, and dispatch it.
 *
 * @param argc Number of command-line arguments.
 * @param argv NULL-terminated command-line argument vector.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on parsing or operation errors.
 */
int main(int argc, char **argv) {
    Command command = {
        .action = ACTION_INVALID,
        .dns_name = NULL,
        .verbose = false
    };
    int parse_result = parseArguments(argc, argv, &command);
    if (parse_result > 0)
        return EXIT_SUCCESS;
    if (parse_result < 0) {
        usage(programName(argv[0]), stderr);
        return EXIT_FAILURE;
    }

    Paths paths;
    if (loadPaths(&paths))
        return EXIT_FAILURE;

    switch (command.action) {
    case ACTION_PREP:
    case ACTION_RESTORE:
        return updateHostsFiles(&paths, command.action, command.verbose);
    case ACTION_ADD:
        return addDnsName(&paths, command.dns_name, command.verbose);
    case ACTION_FLUSH:
        return flushDnsCache(command.verbose);
    case ACTION_INVALID:
        break;
    }

    REPORT_ERROR("unrecognized action");
    return EXIT_FAILURE;
}
