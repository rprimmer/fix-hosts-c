#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "actions.h"
#include "fix-hosts.h"
#include "system-actions.h"

int main(int argc, char **argv) {
    char *program = basename(argv[0]);
    char *DNS_NAME = NULL;
    Action action = ACTION_INVALID;

#ifdef DEBUG
    fprintf(stderr, "argc: %d, optind: %d, line: %d\n", argc, optind, __LINE__);
#endif // DEBUG

    // Handle switches
    int option = 0;
    int option_index = 0;
    static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                           {"flush", no_argument, 0, 'f'},
                                           {"add", required_argument, 0, 'a'},
                                           {0, 0, 0, 0}};
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
            if (optarg && *optarg != '\0') {
                DNS_NAME = strdup(optarg);
                if (!DNS_NAME)
                    handleError("memory allocation failed for DNS name");
            }
            break;
        default:
            handleError("invalid switch provided");
        }
    }

#ifdef DEBUG
    fprintf(stderr, "argc: %d, optind: %d, line: %d\n", argc, optind, __LINE__);
#endif // DEBUG

    // Handle arguments
    const char *const PREP = "prep";
    const char *const RESTORE = "restore";
    char *argument = NULL;
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
    int retval = 0;
    switch (action) {
    case ACTION_PREP:
        retval = updateHostsFiles(HOSTS, HOSTS_ORIG, action);
        break;
    case ACTION_RESTORE:
        retval = updateHostsFiles(HOSTS_ORIG, HOSTS, action);
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
