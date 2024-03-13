/**
 * @file fix-hosts.c
 * @author Robert Primmer (https://github.com/rprimmer)
 * @brief Modify /etc/hosts for use with hblock(1)
 * @details hblock(1) is a shell script, available on 
 * [homebrew](https://brew.sh), that blocks ads, beacons and malware sites. It
 * does this by editing `/etc/hosts` and setting the IP address for such sites
 * to 0.0.0.0. The issue is that hblock sometimes adds sites to /etc/hosts that
 * are needed. This executable fixes such issues by adding good DNS hosts to
 * the exclusion list (`/etc/hblock/allow.list`) and removing the corresponding
 * entry from `/etc/hosts`. It will also optionally flush the DNS cache and 
 * restart the `mDNSResponder` daemon.
 * @version 1.0
 * @date 2024-03-10
 */

#include "fix-hosts.h"

/**
 * @brief Display usage
 * 
 * @param program Basename of calling program
 */
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

/**
 * @brief Modify /etc/hosts 
 * 
 * @param src Source file for action, typically hosts or hosts-ORIG
 * @param dst Destination for action, typically hosts or hosts-ORIG
 * @param action Action being performed. ACTION_PREP is action of interest.
 * @return int Return status
 */
int updateHostsFiles(const char *src, const char *dst, Action action) {

#ifdef DEBUG
    fprintf(stderr, "In function updateHostsFiles, src: %s, dst: %s, action: %d, line %d\n", src, dst, action, __LINE__);
    fprintf(stderr, "HOSTS: %s, HOSTS_ORIG: %s\n", HOSTS, HOSTS_ORIG);
#endif // DEBUG

    uid_t original_uid = getuid();

    if (access(src, F_OK))
        handleError("source file not found: %s", src);

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

    // Root typically required to edit files in /etc. For this action user must call with sudo(1)
    if (setuid(0) == -1)
        handleError("setuid to root failed");

    if (copyFile(src, dst))
        handleError("unable to copy %s to %s", src, dst);

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

/**
 * @brief Add DNS name to hblock allow list
 * 
 * @param hblock_dir Directory holding hblock allow list
 * @param dns_name DNS name to add
 * @param allow_file Name of allow list file
 * @return int Return status 
 */
int addDnsName(const char *hblock_dir, const char *dns_name, const char *allow_file) {

#ifdef DEBUG
    fprintf(stderr, "In function addDnsName, DNS name: %s, Line: %d\n", dns_name, __LINE__);
#endif // DEBUG

    if (validateDNSname(dns_name))
        handleError("invalid DNS name: %s", dns_name); 

    // If the hblock directory doesn't exist, create it
    struct stat st;
    if (stat(hblock_dir, &st) == -1) {
        if (mkdir(hblock_dir, 0755) == -1) 
            handleError("unable to make dir: %s", hblock_dir);
     } else if (!S_ISDIR(st.st_mode)) 
        handleError("%s is not a directory", hblock_dir);

    // Check if DNS entry already exists in the allow list
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int found = 0;

    if ((file = fopen(allow_file, "r+")) == NULL)
        handleError("failed to open allow list file: %s", allow_file);

    while ((read = getline(&line, &len, file)) != -1) {
        line[strcspn(line, "\n")] = '\0'; // Newline character messes up strcmp()

        if (strcmp(line, dns_name) == 0) {
            printf("DNS entry %s already exists in %s\n", dns_name, allow_file);
            found = 1;
            break;
        }
    }
    
    if (!found) 
        fprintf(file, "%s\n", dns_name);

    rewind(file);
    printf("Contents of %s:\n", allow_file);
    while ((read = getline(&line, &len, file)) != -1) 
        printf("%s", line);

    fclose(file);
    free(line);

    // Running hblock(1) updates /etc/hosts sans DNS names in allow list
    printf("Running hblock(1) to update hosts file\n");
    // if (system(HBLOCK))
    //     handleError("hblock failed");

    return EXIT_SUCCESS;
}

/**
 * @brief Flush the DNS cache and restart mDNSResponder daemon 
 *
 * @return int Return status 
 */
int dnsFlush(void) {

#ifdef DEBUG
    fprintf(stderr, "In function dnsFlush, Line: %d\n", __LINE__);
#endif // DEBUG

    struct utsname system_info;      
    int result = uname(&system_info);

    if (result == 0 && strcmp(system_info.sysname, "Darwin")) 
        handleError("flush action is specific to macOS\n");

    printf("Flushing DNS cache...\n");
    if (system("dscacheutil -flushcache"))
        handleError("dschacheutil failed");

    sleep(3);

    const char *service_name = "mDNSResponder";

#ifdef DEBUG
    fprintf(stderr, "PIDs for %s before restart\n", service_name);
    if (checkProcess(service_name)) handleError("checkProcess failed");
    if (displayProcess(service_name)) handleError("displayProcess failed");
#endif // DEBUG

    printf("Restarting the %s service…\n", service_name);
    char command[30];
    snprintf(command, sizeof(command), "%s %s", "killall", service_name);
    if (system(command))
        handleError("%s failed", command);

    sleep(3); 

    if (checkProcess(service_name))
        handleError("checkProcess failed");

    if (displayProcess(service_name))
        handleError("displayProcess failed");

    return EXIT_SUCCESS;
}