/**
 * @file fix-hosts.c
 * @brief Application-level hosts-file operations.
 */

#include "fix-hosts.h"
#include "system-actions.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

/**
 * Join one directory and leaf name with exactly one slash.
 *
 * @param destination Buffer that receives the joined path.
 * @param capacity Size of destination in bytes.
 * @param directory Directory portion of the path.
 * @param name Final pathname component.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE when the path does not fit.
 */
static int joinPath(char *destination, size_t capacity,
                    const char *directory, const char *name) {
    size_t directory_length = strlen(directory);
    if (directory_length == 0) {
        REPORT_ERROR("directory path cannot be empty");
        return EXIT_FAILURE;
    }

    int length = snprintf(destination, capacity, "%s%s%s", directory,
                          directory[directory_length - 1] == '/' ? "" : "/", name);
    if (length < 0 || (size_t)length >= capacity) {
        REPORT_ERROR("path is too long: %s/%s", directory, name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void usage(const char *program, FILE *stream) {
    fprintf(stream,
            "Usage: %s [OPTIONS] <ACTION>\n\n"
            "Options:\n"
            "  -h, --help            Display this help message and exit.\n"
            "  -v, --verbose         Display detailed operation and file information.\n"
            "  -a, --add DNS_NAME    Add a DNS entry to the allow list and unblock it.\n"
            "  -f, --flush           Flush the DNS cache and restart mDNSResponder.\n\n"
            "Actions:\n"
            "  prep                  Back up the hosts file and run hblock.\n"
            "  restore               Reinstate the original hosts file.\n\n"
            "Only one action may be supplied at a time. Changes beneath /etc require\n"
            "running the command with suitable privileges. Flush is macOS-specific.\n",
            program);
}

int loadPaths(Paths *paths) {
    const char *etc_dir = getenv("FIX_HOSTFILES_ETC_DIR");
    const char *hblock_dir = getenv("FIX_HOSTFILES_HBLOCK_DIR");

    if (etc_dir == NULL || *etc_dir == '\0')
        etc_dir = "/etc";

    int length = snprintf(paths->etc_dir, sizeof(paths->etc_dir), "%s", etc_dir);
    if (length < 0 || (size_t)length >= sizeof(paths->etc_dir) ||
        joinPath(paths->hosts_file, sizeof(paths->hosts_file), etc_dir, "hosts") ||
        joinPath(paths->original_hosts_file, sizeof(paths->original_hosts_file),
                 etc_dir, "hosts-ORIG"))
        return EXIT_FAILURE;

    if (hblock_dir != NULL && *hblock_dir != '\0') {
        length = snprintf(paths->hblock_dir, sizeof(paths->hblock_dir), "%s",
                          hblock_dir);
        if (length < 0 || (size_t)length >= sizeof(paths->hblock_dir))
            return EXIT_FAILURE;
    } else if (joinPath(paths->hblock_dir, sizeof(paths->hblock_dir),
                        etc_dir, "hblock")) {
        return EXIT_FAILURE;
    }

    return joinPath(paths->allow_file, sizeof(paths->allow_file),
                    paths->hblock_dir, "allow.list");
}

/**
 * Display metadata for a path when it exists.
 *
 * @param label Heading that identifies the path's role.
 * @param path Filesystem path to inspect.
 * @return EXIT_SUCCESS when absent or successfully inspected; otherwise
 *         EXIT_FAILURE.
 */
static int showFileInfo(const char *label, const char *path) {
    if (!fileExists(path))
        return EXIT_SUCCESS;

    printf("\n%s\n", label);
    return fileInfo(path);
}

int updateHostsFiles(const Paths *paths, Action action, bool verbose) {
    const char *source = action == ACTION_PREP
                             ? paths->hosts_file
                             : paths->original_hosts_file;
    const char *destination = action == ACTION_PREP
                                  ? paths->original_hosts_file
                                  : paths->hosts_file;

    if (!fileExists(source)) {
        REPORT_ERROR("required source file does not exist: %s", source);
        return EXIT_FAILURE;
    }

    puts("Existing hosts files:");
    if (listFiles(paths->etc_dir, "hosts*"))
        return EXIT_FAILURE;

    if (verbose && showFileInfo("Source file details:", source))
        return EXIT_FAILURE;
    if (verbose && showFileInfo("Existing destination details:", destination))
        return EXIT_FAILURE;

    if (fileExists(destination)) {
        printf("\nWARNING: %s already exists and will be overwritten.\n\n",
               destination);
        if (!confirm("Do you want to continue?")) {
            puts("Exiting...");
            return EXIT_SUCCESS;
        }
    }

    if (copyFile(source, destination))
        return EXIT_FAILURE;

    if (action == ACTION_PREP) {
        char *const arguments[] = {"hblock", NULL};
        puts("Running hblock to update the hosts file.");
        int status = runCommand(arguments);
        if (status != EXIT_SUCCESS) {
            REPORT_ERROR("hblock failed with exit status %d", status);
            return EXIT_FAILURE;
        }
    }

    puts("Hosts file updated. New hosts files:");
    if (listFiles(paths->etc_dir, "hosts*"))
        return EXIT_FAILURE;

    if (verbose &&
        (showFileInfo("Active hosts file details:", paths->hosts_file) ||
         showFileInfo("Original hosts backup details:",
                      paths->original_hosts_file)))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/**
 * Determine whether a hosts-file line contains an exact DNS token.
 *
 * Tokens are separated by whitespace; comments beginning with # are ignored.
 *
 * @param line Complete hosts-file line.
 * @param dns_name DNS token to locate.
 * @return true when an exact token is present before any comment.
 */
static bool lineContainsHost(const char *line, const char *dns_name) {
    size_t expected_length = strlen(dns_name);
    const unsigned char *cursor = (const unsigned char *)line;

    while (*cursor != '\0') {
        while (isspace(*cursor))
            cursor++;
        if (*cursor == '\0' || *cursor == '#')
            break;

        const unsigned char *token = cursor;
        while (*cursor != '\0' && !isspace(*cursor) && *cursor != '#')
            cursor++;

        size_t token_length = (size_t)(cursor - token);
        if (token_length == expected_length &&
            strncmp((const char *)token, dns_name, expected_length) == 0)
            return true;
    }
    return false;
}

int removeHostEntry(const char *hosts_path, const char *dns_name) {
    struct stat hosts_status;
    char backup_path[PATH_MAX];
    char temporary_path[PATH_MAX];
    FILE *source = NULL;
    FILE *destination = NULL;
    char *line = NULL;
    size_t capacity = 0;
    int temporary_fd = -1;
    int result = EXIT_FAILURE;

    if (stat(hosts_path, &hosts_status) == -1) {
        REPORT_ERROR("stat: %s: %s", hosts_path, strerror(errno));
        return result;
    }

    int backup_length =
        snprintf(backup_path, sizeof(backup_path), "%s.bak", hosts_path);
    int temporary_length =
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp.XXXXXX", hosts_path);
    if (backup_length < 0 || (size_t)backup_length >= sizeof(backup_path) ||
        temporary_length < 0 ||
        (size_t)temporary_length >= sizeof(temporary_path)) {
        REPORT_ERROR("hosts path is too long: %s", hosts_path);
        return result;
    }

    if (copyFile(hosts_path, backup_path))
        return result;

    source = fopen(hosts_path, "r");
    if (source == NULL) {
        REPORT_ERROR("fopen: %s: %s", hosts_path, strerror(errno));
        goto cleanup;
    }

    temporary_fd = mkstemp(temporary_path);
    if (temporary_fd == -1) {
        REPORT_ERROR("mkstemp: %s", strerror(errno));
        goto cleanup;
    }

    destination = fdopen(temporary_fd, "w");
    if (destination == NULL) {
        REPORT_ERROR("fdopen: %s", strerror(errno));
        goto cleanup;
    }
    temporary_fd = -1;

    while (getline(&line, &capacity, source) != -1) {
        if (!lineContainsHost(line, dns_name) && fputs(line, destination) == EOF) {
            REPORT_ERROR("writing temporary hosts file: %s", strerror(errno));
            goto cleanup;
        }
    }
    if (ferror(source)) {
        REPORT_ERROR("reading hosts file: %s", strerror(errno));
        goto cleanup;
    }

    if (fflush(destination) == EOF ||
        fsync(fileno(destination)) == -1 ||
        fchmod(fileno(destination), hosts_status.st_mode & 07777) == -1) {
        REPORT_ERROR("finalizing temporary hosts file: %s", strerror(errno));
        goto cleanup;
    }
    if (geteuid() == 0 &&
        fchown(fileno(destination), hosts_status.st_uid, hosts_status.st_gid) == -1) {
        REPORT_ERROR("preserving hosts-file ownership: %s", strerror(errno));
        goto cleanup;
    }

    if (fclose(destination) == EOF) {
        destination = NULL;
        REPORT_ERROR("closing temporary hosts file: %s", strerror(errno));
        goto cleanup;
    }
    destination = NULL;

    if (rename(temporary_path, hosts_path) == -1) {
        REPORT_ERROR("rename: %s: %s", hosts_path, strerror(errno));
        goto cleanup;
    }

    result = EXIT_SUCCESS;

cleanup:
    free(line);
    if (source != NULL)
        fclose(source);
    if (destination != NULL)
        fclose(destination);
    if (temporary_fd != -1)
        close(temporary_fd);
    if (result != EXIT_SUCCESS)
        unlink(temporary_path);
    return result;
}

int addDnsName(const Paths *paths, const char *dns_name, bool verbose) {
    if (!isValidDnsName(dns_name)) {
        REPORT_ERROR("invalid DNS name: %s",
                     dns_name == NULL ? "(null)" : dns_name);
        return EXIT_FAILURE;
    }
    if (ensureDirectory(paths->hblock_dir))
        return EXIT_FAILURE;

    if (verbose && showFileInfo("Hosts file before update:", paths->hosts_file))
        return EXIT_FAILURE;

    printf("Adding %s to %s\n", dns_name, paths->allow_file);
    if (fileContainsLine(paths->allow_file, dns_name)) {
        printf("DNS entry %s already exists in %s\n", dns_name,
               paths->allow_file);
    } else if (appendLine(paths->allow_file, dns_name)) {
        return EXIT_FAILURE;
    }

    printf("Removing %s from %s\n", dns_name, paths->hosts_file);
    if (removeHostEntry(paths->hosts_file, dns_name))
        return EXIT_FAILURE;

    if (verbose &&
        (showFileInfo("Allow-list details:", paths->allow_file) ||
         showFileInfo("Hosts file after update:", paths->hosts_file)))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int flushDnsCache(bool verbose) {
    struct utsname system_information;
    if (uname(&system_information) == -1) {
        REPORT_ERROR("uname: %s", strerror(errno));
        return EXIT_FAILURE;
    }
    if (strcmp(system_information.sysname, "Darwin") != 0) {
        REPORT_ERROR("flush action is specific to macOS");
        return EXIT_FAILURE;
    }

    char *const flush_arguments[] = {"dscacheutil", "-flushcache", NULL};
    char *const restart_arguments[] = {"killall", "-HUP", "mDNSResponder", NULL};
    char *const inspect_arguments[] = {"pgrep", "-fl", "mDNSResponder", NULL};

    if (verbose) {
        printf("Platform: %s %s\n", system_information.sysname,
               system_information.release);
        puts("Commands: dscacheutil -flushcache; killall -HUP mDNSResponder; "
             "pgrep -fl mDNSResponder");
    }

    puts("Flushing DNS cache...");
    if (runCommand(flush_arguments))
        return EXIT_FAILURE;
    sleep(4);

    puts("Restarting the mDNSResponder service...");
    if (runCommand(restart_arguments))
        return EXIT_FAILURE;
    sleep(4);

    if (runCommand(inspect_arguments)) {
        fputs("Warning: mDNSResponder is not running.\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
