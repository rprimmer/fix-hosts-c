#include <stdio.h>
#include <stdlib.h>

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

int prepHostsFile(void) { 
    printf("In function prepHostsFile\n");
    return EXIT_SUCCESS; 
}

int copyHostsFiles(void) {
    printf("In function copyHostsFile\n");
    return EXIT_SUCCESS;
}

int restoreHostsFile(void) {
    printf("In function restoreHostsFile\n");
    return EXIT_SUCCESS;
}

int addDnsName(char *dns_name) {
    printf("In function addDnsName, DNS name: %s\n", dns_name);
    return EXIT_SUCCESS;
}

int dnsFlush(void) {
    printf("In function dnsFlush\n");
    return EXIT_SUCCESS;
}