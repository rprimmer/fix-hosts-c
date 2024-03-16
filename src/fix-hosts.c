// fix-hosts.c

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
    printf("  sudo %s prep\n", program);
    printf("  sudo %s restore\n", program);
    printf("  sudo %s -a example.domain.com\n", program);
    printf("  sudo %s -f\n\n", program);
    printf("Restrictions:\n");
    printf("  Requires privileged actions. User must know sudo(1) password.\n");
    printf("  The flush action is specific to macOS.\n\n");
    printf("Notes:\n");
    printf("  When adding a DNS entry (-a) or flushing the cache (-f) the arguments <prep> | <restore> are not required\n");
    exit(EXIT_SUCCESS);
} // usage()

int updateHostsFiles(const char *src, const char *dst, Action action) {

#ifdef DEBUG
    fprintf(stderr, "%s, %d: function updateHostsFiles, src: %s, dst: %s, action: %d\n", basename(__FILE__), __LINE__, src, dst, action);
    fprintf(stderr, "%s, %d: HOSTS: %s, HOSTS_ORIG: %s\n", basename(__FILE__), __LINE__, HOSTS, HOSTS_ORIG);
#endif // DEBUG

    uid_t original_uid = getuid();

    if (access(src, F_OK))
        HANDLE_ERROR("source file not found: %s", src); 

    printf("Existing hosts files...\n");
    if (lsFiles(ETC, HOSTFILES))
        HANDLE_ERROR("unable to access host files"); 

    if (access(dst, F_OK) == 0) {
        printf("\nWARNING: File %s already exists. This action will overwrite that file\n", dst);
        if (!booleanQuery("Do you want to continue? (y/n)")) {
            printf("Exiting...\n");
            return EXIT_SUCCESS;
        }
    }

    // Root typically required to edit files in /etc. For this action user must call with sudo(1)
    if (setuid(0) == -1)
        HANDLE_ERROR("setuid to root failed"); 

    if (copyFile(src, dst))
        HANDLE_ERROR("unable to copy %s to %s", src, dst); 

    if (setuid(original_uid) == -1)
        HANDLE_ERROR("setuid to original user failed"); 

    if (action == ACTION_PREP) {
        printf("Running hblock(1) to update hosts file\n");
        if (system(HBLOCK))
            HANDLE_ERROR("hblock failed"); 
    }

    printf("Hosts file has been updated.\n");
    printf("Here are the new hosts files\n");
    lsFiles(ETC, HOSTFILES);

    return EXIT_SUCCESS;
} // updateHostsFiles()

int addDnsName(const char *hblock_dir, const char *dns_name, const char *allow_file) {
#ifdef DEBUG
    fprintf(stderr, "%s, %d: function addDnsName, DNS name: %s\n", basename(__FILE__), __LINE__, dns_name);
#endif // DEBUG

    if (validateDNSname(dns_name))
        HANDLE_ERROR("invalid DNS name: %s", dns_name);

    // If the hblock directory doesn't exist, create it
    struct stat st;
    if (stat(hblock_dir, &st) == -1) {
        if (mkdir(hblock_dir, 0755) == -1) 
            HANDLE_ERROR("unable to create dir: %s", hblock_dir); 
     } else if (!S_ISDIR(st.st_mode)) 
        HANDLE_ERROR("%s is not a directory", hblock_dir); 

    // Check if DNS entry already exists in the allow list
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int found = 0;

    if ((file = fopen(allow_file, "r+")) == NULL)
        HANDLE_ERROR("failed to open allow list file: %s", allow_file); 

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
    if (system(HBLOCK))
        HANDLE_ERROR("hblock failed"); 

    return EXIT_SUCCESS;
} // addDnsName()

int dnsFlush(void) {
#ifdef DEBUG
    fprintf(stderr, "%s, %d: function dnsFlush\n", basename(__FILE__), __LINE__);
#endif // DEBUG

    struct utsname system_info;      
    int result = uname(&system_info);

    if (result == 0 && strcmp(system_info.sysname, "Darwin")) 
        HANDLE_ERROR("flush action is specific to macOS");

    printf("Flushing DNS cache...\n");
    if (system("dscacheutil -flushcache"))
        HANDLE_ERROR("dschacheutil failed");

    sleep(3);

    const char *service_name = "mDNSResponder";

#ifdef DEBUG
    fprintf(stderr, "%s, %d: PIDs for %s before restart\n", basename(__FILE__), __LINE__, service_name);
    if (checkProcess(service_name)) HANDLE_ERROR("checkProcess failed");
    if (displayProcess(service_name)) HANDLE_ERROR("displayProcess failed");
#endif // DEBUG

    printf("Restarting the %s service…\n", service_name);
    char command[30];
    snprintf(command, sizeof(command), "%s %s", "killall", service_name);
    if (system(command))
        HANDLE_ERROR("%s failed", command);

    sleep(3); 

    if (checkProcess(service_name))
        HANDLE_ERROR("checkProcess failed");

    if (displayProcess(service_name))
        HANDLE_ERROR("displayProcess failed"); 

    return EXIT_SUCCESS;
} // dnsFlush()