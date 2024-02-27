#include "fix-hosts.h"

const char *const ETC = "/etc/";
const char *const HOSTS = "/etc/hosts";
const char *const HOSTS_ORIG = "/etc/hosts-ORIG"; 
const char *const HOSTFILES = "hosts*";
const char *const HBLOCK = "hblock";
const char *const ALLOWLIST = "allow.list";
// const char *const ALLOWLIST = "/etc/hblock/allow.list";

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
    printf("  Requires privileged actions. User must know sudo(1) password\n");
    printf("  The flush action is specific to macOS only.\n\n");
    printf("Notes:\n");
    printf("  When adding a DNS entry (-a) or flushing the cache (-f) the arguments <prep> | <restore> are not required\n");
    exit(EXIT_SUCCESS);
}

int copyHostsFiles(void) {
#ifdef DEBUG
    printf("In function copyHostsFile\n");
#endif // DEBUG

    uid_t original_uid = getuid(); // Store original user ID

    // Check for "hosts" file existence
    if (access(HOSTS, F_OK) != 0) 
        handleError("no hosts file found");  

    // Show user existing /etc/hosts* files 
    printf("Existing hosts files...\n");
    if (lsFiles(ETC, HOSTFILES))
        handleError("unable to access hosts files"); 

    // Check for "hosts-ORIG" file and prompt for overwrite
    if (access(HOSTS_ORIG, F_OK) == 0) {
        printf("\nWARNING: File %s already exists. This action will overwrite that file\n", HOSTS_ORIG);
        if (!booleanQuery("Do you want to continue? (y/n)")) {
            printf("Exiting...\n");
            exit(EXIT_SUCCESS);
        }
    }

    // Copy "hosts" to "hosts-ORIG" using sudo for privileged access
    if (setuid(0) == -1) 
        handleError("setuid to root failed");

    // TODO Add call to either copyFile() or copyFile2()

    if (setuid(original_uid) == -1) 
        handleError("setuid to original user failed");

    // hblock(1) creates a new /etc/hosts file
    printf("Running hblock(1) to update hosts file\n"); 
    // if (system(HBLOCK) != 0)
    //     handleError("hblock failed");

    printf("Hosts file has been updated.\n");
    printf("Here are the new hosts files\n");
    lsFiles(ETC, HOSTFILES);

    return EXIT_SUCCESS;
}


int restoreHostsFile(void) {
#ifdef DEBUG
    printf("In function restoreHostsFile\n");
#endif // DEBUG

    return EXIT_SUCCESS;
}

int addDnsName(const char *dns_name) {
#ifdef DEBUG
    printf("In function addDnsName, DNS name: %s\n", dns_name);
#endif // DEBUG

    return EXIT_SUCCESS;
}

int dnsFlush(void) {
#ifdef DEBUG
    printf("In function dnsFlush\n");
#endif // DEBUG

    return EXIT_SUCCESS;
}