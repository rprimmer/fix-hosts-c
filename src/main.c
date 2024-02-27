#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fix-hosts.h"
#include "system-actions.h"

int main(int argc, char **argv) {
    int option = 0;
    int option_index = 0;
    int retval = 0;
    char *program = basename(argv[0]);
    char *argument = NULL;
    char *DNS_NAME = NULL;
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
    while ((option = getopt_long(argc, argv, "hfa:", long_options, &option_index)) != -1) {
        switch (option) {
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
                handleError("memory allocation failed for DNS name");
            break;
        default:
            handleError("invalid switch provided");
        }
    }

#ifdef DEBUG
    fprintf(stderr, "argc: %d, optind: %d\n", argc, optind);
#endif // DEBUG

    // Handle arguments
    const char *const PREP = "prep";
    const char *const RESTORE = "restore";
    if (argc <= optind && action != ACTION_ADD && action != ACTION_FLUSH)
        handleError("no action specified");
    else if (argc > optind) {
        argument = argv[optind];
        if (strcmp(argument, PREP) == 0)
            action = ACTION_PREP;
        else if (strcmp(argument, RESTORE) == 0)
            action = ACTION_RESTORE;
        else
            handleError("invalid action specified");
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
        handleError("no valid action specified");
    }

    return retval;

} // main()
