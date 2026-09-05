/**
 * @file system-actions.c
 * @brief Filesystem, subprocess, validation, and reporting utilities.
 */

#include "system-actions.h"

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <grp.h>
#include <pwd.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void reportError(const char *file, const char *function, int line,
                 const char *format, ...) {
    const char *filename = strrchr(file, '/');
    filename = filename == NULL ? file : filename + 1;

    fprintf(stderr, "Error in %s:%s, line %d: ", filename, function, line);

    va_list arguments;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fputc('\n', stderr);
}

bool confirm(const char *prompt) {
    char response[16];

    for (;;) {
        printf("%s [y/n]: ", prompt);
        if (fgets(response, sizeof(response), stdin) == NULL)
            return false;

        if (response[0] == 'y' || response[0] == 'Y')
            return true;
        if (response[0] == 'n' || response[0] == 'N')
            return false;

        puts("Invalid response. Please enter 'y' or 'n'.");
    }
}

bool fileExists(const char *path) {
    return access(path, F_OK) == 0;
}

int copyFile(const char *source_path, const char *destination_path) {
    unsigned char buffer[BUFSIZ];
    struct stat source_status;
    FILE *source = NULL;
    FILE *destination = NULL;
    int result = EXIT_FAILURE;

    if (stat(source_path, &source_status) == -1) {
        REPORT_ERROR("stat: %s: %s", source_path, strerror(errno));
        return result;
    }

    source = fopen(source_path, "rb");
    if (source == NULL) {
        REPORT_ERROR("fopen: %s: %s", source_path, strerror(errno));
        return result;
    }

    destination = fopen(destination_path, "wb");
    if (destination == NULL) {
        REPORT_ERROR("fopen: %s: %s", destination_path, strerror(errno));
        goto cleanup;
    }

    for (;;) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), source);
        if (bytes_read > 0 &&
            fwrite(buffer, 1, bytes_read, destination) != bytes_read) {
            REPORT_ERROR("fwrite: %s: %s", destination_path, strerror(errno));
            goto cleanup;
        }

        if (bytes_read < sizeof(buffer)) {
            if (ferror(source)) {
                REPORT_ERROR("fread: %s: %s", source_path, strerror(errno));
                goto cleanup;
            }
            break;
        }
    }

    if (fflush(destination) == EOF ||
        chmod(destination_path, source_status.st_mode & 07777) == -1) {
        REPORT_ERROR("finalizing %s: %s", destination_path, strerror(errno));
        goto cleanup;
    }

    result = EXIT_SUCCESS;

cleanup:
    if (destination != NULL && fclose(destination) == EOF)
        result = EXIT_FAILURE;
    if (fclose(source) == EOF)
        result = EXIT_FAILURE;
    return result;
}

int listFiles(const char *directory_path, const char *pattern) {
    DIR *directory = opendir(directory_path);
    if (directory == NULL) {
        REPORT_ERROR("opendir: %s: %s", directory_path, strerror(errno));
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    int result = EXIT_SUCCESS;

    while ((entry = readdir(directory)) != NULL) {
        if (fnmatch(pattern, entry->d_name, 0) != 0)
            continue;

        char full_path[4096];
        int length = snprintf(full_path, sizeof(full_path), "%s%s%s",
                              directory_path,
                              directory_path[strlen(directory_path) - 1] == '/' ? "" : "/",
                              entry->d_name);
        if (length < 0 || (size_t)length >= sizeof(full_path)) {
            REPORT_ERROR("path is too long: %s/%s", directory_path, entry->d_name);
            result = EXIT_FAILURE;
            break;
        }

        struct stat status;
        if (stat(full_path, &status) == -1) {
            REPORT_ERROR("stat: %s: %s", full_path, strerror(errno));
            result = EXIT_FAILURE;
            break;
        }

        printf("%s (%lld bytes)\n", full_path, (long long)status.st_size);
    }

    if (closedir(directory) == -1) {
        REPORT_ERROR("closedir: %s: %s", directory_path, strerror(errno));
        result = EXIT_FAILURE;
    }
    return result;
}

/**
 * Return a readable name for a mode's filesystem-object type.
 *
 * @param mode st_mode value returned by lstat(2).
 * @return Static string describing the object type.
 */
static const char *fileType(mode_t mode) {
    if (S_ISREG(mode))
        return "regular file";
    if (S_ISDIR(mode))
        return "directory";
    if (S_ISLNK(mode))
        return "symbolic link";
    if (S_ISCHR(mode))
        return "character device";
    if (S_ISBLK(mode))
        return "block device";
    if (S_ISFIFO(mode))
        return "FIFO";
    if (S_ISSOCK(mode))
        return "socket";
    return "unknown";
}

/**
 * Format a mode as the familiar ten-character ls-style permission string.
 *
 * @param mode st_mode value returned by lstat(2).
 * @param permissions Output buffer with room for eleven bytes.
 */
static void formatPermissions(mode_t mode, char permissions[11]) {
    permissions[0] = S_ISDIR(mode)   ? 'd'
                     : S_ISLNK(mode) ? 'l'
                     : S_ISCHR(mode) ? 'c'
                     : S_ISBLK(mode) ? 'b'
                     : S_ISFIFO(mode) ? 'p'
                     : S_ISSOCK(mode) ? 's'
                                      : '-';
    permissions[1] = mode & S_IRUSR ? 'r' : '-';
    permissions[2] = mode & S_IWUSR ? 'w' : '-';
    permissions[3] = mode & S_ISUID ? (mode & S_IXUSR ? 's' : 'S')
                                      : (mode & S_IXUSR ? 'x' : '-');
    permissions[4] = mode & S_IRGRP ? 'r' : '-';
    permissions[5] = mode & S_IWGRP ? 'w' : '-';
    permissions[6] = mode & S_ISGID ? (mode & S_IXGRP ? 's' : 'S')
                                      : (mode & S_IXGRP ? 'x' : '-');
    permissions[7] = mode & S_IROTH ? 'r' : '-';
    permissions[8] = mode & S_IWOTH ? 'w' : '-';
    permissions[9] = mode & S_ISVTX ? (mode & S_IXOTH ? 't' : 'T')
                                      : (mode & S_IXOTH ? 'x' : '-');
    permissions[10] = '\0';
}

int fileInfo(const char *path) {
    struct stat status;
    if (lstat(path, &status) == -1) {
        REPORT_ERROR("lstat: %s: %s", path, strerror(errno));
        return EXIT_FAILURE;
    }

    char permissions[11];
    char modified[32] = "unavailable";
    formatPermissions(status.st_mode, permissions);

    struct tm local_time;
    if (localtime_r(&status.st_mtime, &local_time) != NULL)
        strftime(modified, sizeof(modified), "%Y-%m-%d %H:%M:%S %Z", &local_time);

    const struct passwd *owner = getpwuid(status.st_uid);
    const struct group *group = getgrgid(status.st_gid);

    printf("File: %s\n", path);
    printf("  Type:        %s\n", fileType(status.st_mode));
    printf("  Permissions: %s (%04o)\n", permissions,
           status.st_mode & 07777);
    printf("  Owner:       %s (%u)\n", owner == NULL ? "unknown" : owner->pw_name,
           (unsigned int)status.st_uid);
    printf("  Group:       %s (%u)\n", group == NULL ? "unknown" : group->gr_name,
           (unsigned int)status.st_gid);
    printf("  Size:        %lld bytes\n", (long long)status.st_size);
    printf("  Inode:       %llu\n", (unsigned long long)status.st_ino);
    printf("  Links:       %u\n", (unsigned int)status.st_nlink);
    printf("  Modified:    %s\n", modified);

    return EXIT_SUCCESS;
}

int ensureDirectory(const char *path) {
    struct stat status;

    if (stat(path, &status) == 0) {
        if (S_ISDIR(status.st_mode))
            return EXIT_SUCCESS;
        REPORT_ERROR("not a directory: %s", path);
        return EXIT_FAILURE;
    }

    if (errno != ENOENT) {
        REPORT_ERROR("stat: %s: %s", path, strerror(errno));
        return EXIT_FAILURE;
    }

    if (mkdir(path, 0755) == -1) {
        REPORT_ERROR("mkdir: %s: %s", path, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int runCommand(char *const arguments[]) {
    pid_t child = fork();
    if (child == -1) {
        REPORT_ERROR("fork: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    if (child == 0) {
        execvp(arguments[0], arguments);
        fprintf(stderr, "Error: execvp %s: %s\n", arguments[0], strerror(errno));
        _exit(127);
    }

    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno == EINTR)
            continue;
        REPORT_ERROR("waitpid: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        REPORT_ERROR("%s terminated by signal %d", arguments[0], WTERMSIG(status));
    return EXIT_FAILURE;
}

bool isValidDnsName(const char *dns_name) {
    static const char pattern[] =
        "^([a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,}$";
    regex_t expression;

    if (dns_name == NULL ||
        regcomp(&expression, pattern, REG_EXTENDED | REG_NOSUB) != 0)
        return false;

    int result = regexec(&expression, dns_name, 0, NULL, 0);
    regfree(&expression);
    return result == 0;
}

bool fileContainsLine(const char *path, const char *expected_line) {
    FILE *file = fopen(path, "r");
    if (file == NULL)
        return false;

    char *line = NULL;
    size_t capacity = 0;
    bool found = false;

    while (getline(&line, &capacity, file) != -1) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, expected_line) == 0) {
            found = true;
            break;
        }
    }

    free(line);
    fclose(file);
    return found;
}

int appendLine(const char *path, const char *line) {
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        REPORT_ERROR("fopen: %s: %s", path, strerror(errno));
        return EXIT_FAILURE;
    }

    int result = fprintf(file, "%s\n", line) < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
    if (fclose(file) == EOF)
        result = EXIT_FAILURE;

    if (result != EXIT_SUCCESS)
        REPORT_ERROR("writing: %s: %s", path, strerror(errno));
    return result;
}
