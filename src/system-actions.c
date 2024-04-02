// system-actions.c

#include "system-actions.h"

void handleError(bool fatal, char *file, const char *func, int line, const char *fmt, ...) {
    fprintf(stderr, "Error in %s:%s, line %d: ", basename(file), func, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    if (fatal) 
        exit(EXIT_FAILURE);
}

int booleanQuery(const char *prompt) {
    char response[10];

    printf("%s ", prompt);

    if (fgets(response, sizeof(response), stdin) == NULL) 
        HANDLE_ERROR("failed to read user response"); 

    return (response[0] == 'y' || response[0] == 'Y');
} // booleanQuery()

int fileExists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
} // fileExists()

int copyFile(const char *src, const char *dest) {
    char buffer[BUFSIZ];
    size_t bytesRead, bytesWritten; 

    FILE *source = fopen(src, "rb");
    if (source == NULL) {
        REPORT_ERROR("fopen: %s, file %s", strerror(errno), src); 
        return EXIT_FAILURE;
    }

    FILE *destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        REPORT_ERROR("fopen: %s, file: %s", strerror(errno), dest); 
        return EXIT_FAILURE;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        bytesWritten = fwrite(buffer, 1, bytesRead, destination);
        if (bytesWritten < bytesRead) {
            fclose(source);
            fclose(destination);
            REPORT_ERROR("fwrite: %s, file: %s", strerror(errno), dest);
            return EXIT_FAILURE;
        }
    }

    if (ferror(source)) {
        fclose(source);
        fclose(destination);
        REPORT_ERROR("read error: %s", src);
        return EXIT_FAILURE;
    } else if (!feof(source)) {
        fclose(source);
        fclose(destination);
        REPORT_ERROR("unexpected end of file: %s", src);
        return EXIT_FAILURE;
    }

    fclose(source);
    fclose(destination);

    return EXIT_SUCCESS;
} // copyFile()

int copyFile2(const char *src, const char *dest) {
    char buffer[BUFSIZ];
    ssize_t bytes_read, bytes_written, total_written;

    int source_fd = open(src, O_RDONLY);
    if (source_fd == -1) {
        REPORT_ERROR("open: %s, file %s", strerror(errno), src);
        return EXIT_FAILURE;
    }

    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        close(source_fd);
        REPORT_ERROR("open: %s, file %s", strerror(errno), dest);
        return EXIT_FAILURE;
    }

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        total_written = 0;
        do {
            bytes_written = write(dest_fd, buffer + total_written, bytes_read - total_written);
            if (bytes_written >= 0) {
                total_written += bytes_written;
            } else {
                close(source_fd);
                close(dest_fd);
                REPORT_ERROR("write: %s, file: %s", strerror(errno), dest);
                return EXIT_FAILURE;
            }
        } while (bytes_read > total_written);
    }

    if (bytes_read == -1) {
        close(source_fd);
        close(dest_fd);
        REPORT_ERROR("read error: %s", src);
        return EXIT_FAILURE;
    }

    close(source_fd);
    close(dest_fd);

    return EXIT_SUCCESS;
} // copyFile2()

int lsFiles(const char *dirname, const char *files) { 
    DIR *dir = opendir(dirname);

    if (dir == NULL) {
        REPORT_ERROR("opendir: %s, file: %s", strerror(errno), dirname); 
        return EXIT_FAILURE; 
    }

    struct dirent *entry;
    struct stat file_stat;
    char full_path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(files, entry->d_name, 0) == 0) {
            
            if (dirname[strlen(dirname) - 1] == '/') 
                snprintf(full_path, sizeof(full_path), "%s%s", dirname, entry->d_name);
            else 
                snprintf(full_path, sizeof(full_path), "%s/%s", dirname, entry->d_name);

            if (lstat(full_path, &file_stat) == 0) {
                printf("%s ", full_path);
                printf("Owner: %s ", getpwuid(file_stat.st_uid)->pw_name);
                printf("Group: %s ", getgrgid(file_stat.st_gid)->gr_name);
                printf("Size: %lld ", (long long)file_stat.st_size);
                printf("Last modified: %s", ctime(&file_stat.st_mtime));
            } else {
                closedir(dir); 
                REPORT_ERROR("lstat: %s, file: %s", strerror(errno), full_path);
                return EXIT_FAILURE;
            }
        }
    }

    return (closedir(dir)); 
} // lsFiles()

int fileInfo(const char *filepath) {
    struct stat fileStat;
    if (lstat(filepath, &fileStat) < 0) {
        REPORT_ERROR("lstat: %s, file: %s", strerror(errno), filepath);
        return EXIT_FAILURE;
    }

    printf("Information for %s\n", filepath);
    printf("---------------------------\n");
    printf("File Size: \t\t%lld bytes\n", (long long)fileStat.st_size);
    printf("Number of Links: \t%lu\n", (unsigned long)fileStat.st_nlink);
    printf("File inode: \t\t%lu\n", (unsigned long)fileStat.st_ino);

    printf("File Permissions: \t");
    printf((S_ISDIR(fileStat.st_mode)) ? "d" : (S_ISLNK(fileStat.st_mode)) ? "l" : (S_ISFIFO(fileStat.st_mode)) ? "p" :
           (S_ISSOCK(fileStat.st_mode)) ? "s" : (S_ISCHR(fileStat.st_mode)) ? "c" : (S_ISBLK(fileStat.st_mode)) ? "b" : "-");
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? ((fileStat.st_mode & S_ISUID) ? "s" : "x") : 
           ((fileStat.st_mode & S_ISUID) ? "S" : "-"));
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? ((fileStat.st_mode & S_ISGID) ? "s" : "x") :
           ((fileStat.st_mode & S_ISGID) ? "S" : "-"));
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? ((fileStat.st_mode & S_ISVTX) ? "t" : "x") :
           ((fileStat.st_mode & S_ISVTX) ? "T" : "-"));
    printf("\n");

    printf("Last access time: \t%s", ctime(&fileStat.st_atime));
    printf("Last modification time: %s", ctime(&fileStat.st_mtime));
    printf("Last status change time: %s", ctime(&fileStat.st_ctime));

    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);
    printf("File Owner: \t\t%s (%d)\n", pw->pw_name, fileStat.st_uid);
    printf("File Group: \t\t%s (%d)\n", gr->gr_name, fileStat.st_gid);
    printf("Block Size: \t\t%ld bytes\n", (long)fileStat.st_blksize);

    printf("File Type: \t\t");
    if (S_ISREG(fileStat.st_mode))
        printf("Regular file\n");
    else if (S_ISDIR(fileStat.st_mode))
        printf("Directory\n");
    else if (S_ISCHR(fileStat.st_mode))
        printf("Character device\n");
    else if (S_ISBLK(fileStat.st_mode))
        printf("Block device\n");
    else if (S_ISFIFO(fileStat.st_mode))
        printf("FIFO\n");
    else if (S_ISLNK(fileStat.st_mode))
        printf("Symbolic link\n");
    else if (S_ISSOCK(fileStat.st_mode))
        printf("Socket\n");
    else
        printf("Unknown\n");

    return EXIT_SUCCESS; 
} // fileInfo()

int checkProcess(const char *process_name, bool display_pids) {
    char command[128];
    snprintf(command, sizeof(command), "pgrep %s", process_name);

    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        REPORT_ERROR("popen: %s, process name: %s", strerror(errno), process_name);
        return EXIT_FAILURE;
    }

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe) == NULL) {
        if (!display_pids) {
            pclose(pipe);
            return EXIT_FAILURE;
        }
        fprintf(stderr, "Warning: the %s process is not running.\n", process_name);
    } else {
        if (!display_pids) {
            pclose(pipe);
            return EXIT_SUCCESS;
        }
        pid_t pid = atoi(strtok(buffer, "\n")); // Extract first PID
        printf("The %s process is running with PID(s): %d", process_name, pid);

        // Check for additional PIDs
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            pid = atoi(strtok(buffer, "\n"));
            printf(" %d", pid);
        }
        printf("\n");
    }

    return (pclose(pipe)); 
} // checkProcess()

int displayProcess(const char *process_name) {
    char command[128];
    snprintf(command, sizeof(command), "ps aux | grep %s | grep -v grep", process_name);

    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        REPORT_ERROR("popen: %s, process name: %s", strerror(errno), process_name);
        return EXIT_FAILURE;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        printf("%s", buffer);

    return (pclose(pipe));
} // displayProcess()

int validateDNSname(const char *dns_name) {
    regex_t regex;
    int result;
    const char *dns_regex = "^([a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,}$";

    // Compile the regular expression
    result = regcomp(&regex, dns_regex, REG_EXTENDED | REG_NOSUB);
    if (result) {
        REPORT_ERROR("regex: %s, DNS name: %s", strerror(errno), dns_name); 
        return EXIT_FAILURE;
    }

    // Execute the regular expression
    result = regexec(&regex, dns_name, 0, NULL, 0);
    regfree(&regex); // Free memory allocated to the pattern buffer by regcomp

    return result; 
} // validateDNSname
