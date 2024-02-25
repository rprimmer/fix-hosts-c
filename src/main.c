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

    if (action == ACTION_ADD) {
        retval = addDnsName(DNS_NAME);
        free(DNS_NAME);
        return retval;
    } else if (action == ACTION_FLUSH) {
        retval = dnsFlush();
        return retval;
    }

    if (argc <= optind)
        handleError("No action specified");

    if (strcmp(argv[optind], "prep") == 0) {
        action = ACTION_PREP;
    } else if (strcmp(argv[optind], "restore") == 0) {
        action = ACTION_RESTORE;
    } else {
        handleError("Invalid action specified");
    }

    switch (action) {
    case ACTION_PREP:
        retval = copyHostsFiles();
        break;
    case ACTION_RESTORE:
        retval = restoreHostsFile();
        break;
    default:
        handleError("No valid action specified");
    }

    return retval;

} // main()
