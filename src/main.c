#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fix-hosts.h"

enum action_t {
    ACTION_PREP,
    ACTION_RESTORE,
    ACTION_ADD,
    ACTION_FLUSH,
    ACTION_INVALID,
};

enum action_t action = ACTION_INVALID;
char *DNS_NAME;

void usage(char *program) {
    printf("Usage: %s [OPTIONS] <ACTION>\n\n", program);
    printf("  -h       Display this help message and exit\n");
    printf("  -f       Flush DNS cache and restart mDNSResponder\n");
    printf("  -a       Add a DNS entry to allow list and remove from "
           "/etc/hosts\n\n");
    printf("Actions:\n");
    printf("  prep     Backup hosts file and run hblock to create a new hosts "
           "file\n");
    printf("  restore  Reinstate original hosts file\n\n");
    printf("Examples:\n");
    printf("  %s prep\n", program);
    printf("  %s restore\n", program);
    printf("  %s -a example.domain.com\n\n", program);
    printf("Restrictions:\n");
    printf("  Requires privileged actions. User must know sudo(1) password");
    printf("  The flush action is specific to macOS only.\n\n");
    printf("Notes:\n");
    printf("  When adding a DNS entry (-a) or flushing the cache (-f) the "
           "arguments 'prep' | 'restore' are not required\n");
    exit(EXIT_SUCCESS);
}

void handleError(char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int opt;
    int retval = 0; 
    char *program = argv[0];

    while (getopts(argc, argv, "fha") != -1) {
        switch (opt) {
        case 'h':
            usage(program);
            return EXIT_SUCCESS; 
            break;
        case 'f':
            action = ACTION_FLUSH;
            printf("DNS Flush\n");
            // retval = dnsFlush();
            return retval; 
            break;
        case 'a':
            action = ACTION_ADD;
            DNS_NAME = strdup(optarg);
            if (!DNS_NAME) handleError("Invalid argument for -a switch");
            printf("DNS Add\n");
            // retval = addDnsName(DNS_NAME);
            return retval; 
            break;
        default:
            action = ACTION_INVALID;
            handle_error("Invalid option provided");
        }
    }

    // At this point the only valid arguments are 'prep' | 'restore'
    if (optind < argc) {
        handleError("Invalid arguments provided");
    }

#ifdef DEBUG
    if (optind >= argc)
        fprintf(stderr, "argc[optind] %s\n", argc[optind]);
#endif

    if (strcmp(argv[optind], "prep")) 
        printf("Arg %d is prep\n", optind);
    else if (strcmp(argv[optind], "restore"))
        printf("Arg %d is restore, optind");
    else
        handleError("Invalid arguments provided");

    // switch (action) {
    // case ACTION_PREP:
    //     copyHostsFile();
    //     break;
    // case ACTION_RESTORE:
    //     restoreHostsFile();
    //     break;
    // default:
    //     // Should not reach here due to error handling in processArguments()
    //     break;
    // }

    free(DNS_NAME);
    return EXIT_SUCCESS;

} // main()
