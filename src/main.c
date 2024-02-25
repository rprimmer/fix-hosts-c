#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fix-hosts.h"

void usage(char *program) {
    printf("Usage: %s [OPTIONS] <ACTION>\n\n", program);
    printf("  -h, --help            Display this help message and exit\n");
    printf("  -a, --add <DNS name>  Add a DNS entry to allow list and remove from /etc/hosts\n");
    printf("  -f, --flush           Flush DNS cache and restart mDNSResponder\n\n");
    printf("Actions:\n");
    printf("  prep     Backup hosts file and run hblock to create a new hosts file\n");
    printf("  restore  Reinstate original hosts file\n\n");
    printf("Examples:\n");
    printf("  %s prep\n", program);
    printf("  %s restore\n", program);
    printf("  %s -a example.domain.com\n", program);
    printf("  %s -f\n\n", program);
    printf("Restrictions:\n");
    printf("  Requires privileged actions. User must know sudo(1) password\n");
    printf("  The flush action is specific to macOS only.\n\n");
    printf("Notes:\n");
    printf("  When adding a DNS entry (-a) or flushing the cache (-f) the arguments <prep> | <restore> are not required\n");
    exit(EXIT_SUCCESS);
}

void handleError(char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(EXIT_FAILURE);
}

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
        // retval = addDnsName(DNS_NAME);
        free(DNS_NAME);
        return retval;
    } else if (action == ACTION_FLUSH) {
        // retval = dnsFlush();
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
        // copyHostsFile();
        break;
    case ACTION_RESTORE:
        // restoreHostsFile();
        break;
    default:
        handleError("No valid action specified");
    }

    return EXIT_SUCCESS;

} // main()
