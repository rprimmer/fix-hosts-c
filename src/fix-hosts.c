#include "fix-hosts.h"

const char *const ETC = "/etc";
const char *const HOSTS = "/etc/hosts";
const char *const HBLOCK = "/etc/hblock";
const char *const ALLOWS = "allow.list";
// const char *const ALLOW = "/etc/hblock/allow.list";

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
    if (access("/etc/hosts", F_OK) != 0) {
        handleError("no hosts file found");
    }

    // Show user existing /etc/hosts* files 
    printf("Existing hosts files...\n"); 

    // TODO Move this function to systems-actions.c 
    DIR *dir = opendir("/etc");

    if (dir == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch("hosts*", entry->d_name, 0) == 0) {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "/etc/%s", entry->d_name);

            if (lstat(full_path, &file_stat) == 0) {
                printf("%s ", full_path);

                // Get and print file information
                printf("Owner: %s ", getpwuid(file_stat.st_uid)->pw_name);
                printf("Group: %s ", getgrgid(file_stat.st_gid)->gr_name);
                printf("Size: %lld ", file_stat.st_size);
                printf("Last modified: %s", ctime(&file_stat.st_mtime));
                // printf("\n"); // Not sure why this isn't needed, but it isn't. 
            } else {
                perror("lstat");
            }
        }
    }

    // Check for "hosts-ORIG" file and prompt for overwrite
    if (access("/etc/hosts-ORIG", F_OK) == 0) {
        printf("\nWARNING: File hosts-ORIG already exists. This action will overwrite that file\n");
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
    // if (system("hblock") != 0)
    //     handleError("hblock failed");

    printf("Hosts file updated. New hosts files\n"); 
    // TODO display hosts

    closedir(dir);

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