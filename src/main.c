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
    char program[PATH_MAX];
    basename_r(argv[0], program); // basename(3) cannot be re-used for persistent vars, cf. manpage.
    char *DNS_NAME = NULL;
    Action action = ACTION_INVALID;

#ifdef DEBUG
    fprintf(stderr, "%s, %d: argc: %d, optind: %d\n", basename(__FILE__), __LINE__, argc, optind);
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
                    HANDLE_ERROR("memory allocation failed for DNS name: %s", DNS_NAME);
            }
            break;
        default:
            HANDLE_ERROR("invalid switch provided"); 
        }
    }

#ifdef DEBUG
    fprintf(stderr, "%s, %d: argc: %d, optind: %d\n", basename(__FILE__), __LINE__, argc, optind);
    fileInfo(ETC);
#endif // DEBUG

    // Handle arguments
    const char *const PREP = "prep";
    const char *const RESTORE = "restore";
    char *argument = NULL;
    if (argc <= optind && action != ACTION_ADD && action != ACTION_FLUSH)
        usage(program);     // no action specified by user at instantiation 
    else if (argc > optind) {
        argument = argv[optind];
        if (strcmp(argument, PREP) == 0)
            action = ACTION_PREP;
        else if (strcmp(argument, RESTORE) == 0)
            action = ACTION_RESTORE;
    }

    // Handle actions
    int retval = 1;
    switch (action) {
    case ACTION_PREP:
        retval = updateHostsFiles(HOSTS, HOSTS_ORIG, action);
        break;
    case ACTION_RESTORE:
        retval = updateHostsFiles(HOSTS_ORIG, HOSTS, action);
        break;
    case ACTION_ADD:
        retval = addDnsName(HBLOCK_DIR, DNS_NAME, ALLOWLIST);
        free(DNS_NAME);
        break;
    case ACTION_FLUSH:
        retval = dnsFlush();
        break;
    default:
        HANDLE_ERROR("no valid action specified"); 
    }

    return retval;
} 
