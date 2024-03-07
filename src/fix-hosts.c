#include "fix-hosts.h"

void usage(const char *program) {
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
    printf("  Requires privileged actions. User must know sudo(1) password.\n");
    printf("  The flush action is specific to macOS.\n\n");
    printf("Notes:\n");
    printf("  When adding a DNS entry (-a) or flushing the cache (-f) the arguments <prep> | <restore> are not required\n");
    exit(EXIT_SUCCESS);
}

int updateHostsFiles(const char *src, const char *dst, Action action) {

#ifdef DEBUG
    fprintf(stderr, "In function updateHostsFiles, src: %s, dst: %s, action: %d\n", src, dst, action);
#endif // DEBUG

    uid_t original_uid = getuid();

    if (access(src, F_OK))
        handleError("source file not found");

    printf("Existing hosts files...\n");
    if (lsFiles(ETC, HOSTFILES))
        handleError("unable to access hosts files");

    if (access(dst, F_OK) == 0) {
        printf("\nWARNING: File %s already exists. This action will overwrite that file\n", dst);
        if (!booleanQuery("Do you want to continue? (y/n)")) {
            printf("Exiting...\n");
            return EXIT_SUCCESS;
        }
    }

    if (setuid(0) == -1)
        handleError("setuid to root failed");

    if (copyFile(src, dst))
        handleError("unable to copy hosts file");

    if (setuid(original_uid) == -1)
        handleError("setuid to original user failed");

    if (action == ACTION_PREP) {
        printf("Running hblock(1) to update hosts file\n");
        // if (system(HBLOCK))
        //     handleError("hblock failed");
    }

    printf("Hosts file has been updated.\n");
    printf("Here are the new hosts files\n");
    lsFiles(ETC, HOSTFILES);

    return EXIT_SUCCESS;
}

int addDnsName(const char *dns_name) {
#ifdef DEBUG
    fprintf(stderr, "In function addDnsName, DNS name: %s\n", dns_name);
#endif // DEBUG

    return EXIT_SUCCESS;
}

int dnsFlush(void) {
#ifdef DEBUG
    fprintf(stderr, "In function dnsFlush\n");
#endif // DEBUG

    return EXIT_SUCCESS;
}