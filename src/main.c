#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fix-hosts.h"

int main(int argc, char **argv) {
    int opt = 0;
    int retval = 0;
    int option_index = 0;
    char *program = basename(argv[0]);
    char *DNS_NAME = NULL;
    char *argument = NULL;
    enum action_t {
        ACTION_PREP,
        ACTION_RESTORE,
        ACTION_ADD,
        ACTION_FLUSH,
        ACTION_INVALID,
    };
    enum action_t action = ACTION_INVALID;
    static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                           {"flush", no_argument, 0, 'f'},
                                           {"add", required_argument, 0, 'a'},
                                           {0, 0, 0, 0}};

#ifdef DEBUG
    fprintf(stderr, "argc: %d, optind: %d\n", argc, optind);
#endif // DEBUG

    // Handle switches
    while ((opt = getopt_long(argc, argv, "hfa:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            usage(program);
            break;
        case 'f':
            action = ACTION_FLUSH;
            break;
        case 'a':
            action = ACTION_ADD;
            DNS_NAME = strdup(optarg);
            if (!DNS_NAME)
                handleError("Memory allocation failed for DNS name");
            break;
        default:
            handleError("Invalid option provided");
        }
    }

#ifdef DEBUG
    fprintf(stderr, "argc: %d, optind: %d\n", argc, optind);
#endif // DEBUG

    // Handle arguments
    if (argc <= optind && action != ACTION_ADD && action != ACTION_FLUSH)
        handleError("No action specified");
    else if (argc > optind) {
        argument = argv[optind];
        if (strcmp(argument, "prep") == 0)
            action = ACTION_PREP;
        else if (strcmp(argument, "restore") == 0)
            action = ACTION_RESTORE;
        else
            handleError("Invalid action specified");
    }

    // Handle actions
    switch (action) {
    case ACTION_PREP:
        retval = copyHostsFiles();
        break;
    case ACTION_RESTORE:
        retval = restoreHostsFile();
        break;
    case ACTION_ADD:
        retval = addDnsName(DNS_NAME);
        free(DNS_NAME);
        break;
    case ACTION_FLUSH:
        retval = dnsFlush();
        break;
    default:
        handleError("No valid action specified");
    }

    return retval;

} // main()
