// system-actions.c

#include "system-actions.h"

void handleError(const char *message, ...) {
    if (errno)
        perror(message);
    else {
        fprintf(stderr, "Error: ");
        va_list args;
        va_start(args, message);
        vfprintf(stderr, message, args);
        va_end(args);
        fprintf(stderr, "\n");
    }

    exit(EXIT_FAILURE);
} // handleError()

int booleanQuery(const char *prompt) {
    char response[10];

    printf("%s ", prompt);

    if (fgets(response, sizeof(response), stdin) == NULL)
        HANDLE_ERROR("failed to read user response"); 

    // Empty newline interpreted as non-yes answer 
    if (response[0] == '\n')     
        return 0;

    return (response[0] == 'y' || response[0] == 'Y');
} // booleanQuery()

int fileExists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
} // fileExists()

int copyFile(const char *src, const char *dest) {
    FILE *source, *destination;
    char buffer[BUFSIZ]; // BUFSIZ from stdio.h 
    size_t bytesRead;

    source = fopen(src, "rb");
    if (source == NULL) 
        HANDLE_ERROR("error opening source file %s", src); 

    destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        HANDLE_ERROR("error opening destination file: %s", dest); 
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) 
        fwrite(buffer, 1, bytesRead, destination);

    fclose(source);
    fclose(destination);

    return EXIT_SUCCESS;
} // copyFile()

int copyFile2(const char *src, const char *dest) {
    int source_fd, dest_fd;
    char buffer[BUFSIZ]; // BUFSIZ from stdio.h
    ssize_t bytes_read, bytes_written;

    source_fd = open(src, O_RDONLY);
    if (source_fd == -1) 
        HANDLE_ERROR("failed to open source file: %s", src); 

    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) 
        HANDLE_ERROR("failed to create destination file: %s", dest); 

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1) 
            HANDLE_ERROR("write error"); 
    }

    if (bytes_read == -1) 
        HANDLE_ERROR("read error"); 

    close(source_fd);
    close(dest_fd);

    printf("File copied successfully!\n");

    return EXIT_SUCCESS;
} // copyFile2()

int lsFiles(const char *dirname, const char *files) { 
#ifdef DEBUG
    fprintf(stderr, "%s, %d: in function lsFiles\n", basename(__FILE__), __LINE__);
#endif // DEBUG

    DIR *dir = opendir(dirname);

    if (dir == NULL)
        return EXIT_FAILURE; 

    struct dirent *entry;
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(files, entry->d_name, 0) == 0) {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s%s", dirname, entry->d_name);

            if (lstat(full_path, &file_stat) == 0) {
                printf("%s ", full_path);
                printf("Owner: %s ", getpwuid(file_stat.st_uid)->pw_name);
                printf("Group: %s ", getgrgid(file_stat.st_gid)->gr_name);
                printf("Size: %lld ", file_stat.st_size);
                printf("Last modified: %s", ctime(&file_stat.st_mtime));
                // printf("\n"); --  Not sure why this newline isn't needed, but it isn't.
            } else 
                return EXIT_FAILURE; 
        }
    }

    return (closedir(dir)); 
} // lsFiles()

int fileInfo(const char *filepath) {
    struct stat fileStat;
    if (stat(filepath, &fileStat) < 0) 
        return EXIT_FAILURE;

    printf("Information for %s\n", filepath);
    printf("---------------------------\n");
    printf("File Size: \t\t%lld bytes\n", fileStat.st_size);
    printf("Number of Links: \t%lu\n", (unsigned long)fileStat.st_nlink);
    printf("File inode: \t\t%lu\n", (unsigned long)fileStat.st_ino);

    printf("File Permissions: \t");
    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");

    printf("The file %s a symbolic link\n", (S_ISLNK(fileStat.st_mode)) ? "is" : "is not");

    char *accessTime = ctime(&fileStat.st_atime);
    printf("Last access time: %s", accessTime);
   
    char *modificationTime = ctime(&fileStat.st_mtime);
    printf("Last modification time: %s", modificationTime);

    char *statusChangeTime = ctime(&fileStat.st_ctime);
    printf("Last status change time: %s", statusChangeTime);

    return EXIT_SUCCESS; 
} // fileInfo()

int checkProcess(const char *process_name) {
    char command[128];
    snprintf(command, sizeof(command), "pgrep %s", process_name);

    FILE *pipe = popen(command, "r");
    if (pipe == NULL)
        return EXIT_FAILURE;

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe) == NULL)
        fprintf(stderr, "Warning: the %s process is not running.\n", process_name);
    else {
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
    if (pipe == NULL)
        return EXIT_FAILURE; 

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
    if (result) 
        HANDLE_ERROR("could not compile regex"); 

    // Execute the regular expression
    result = regexec(&regex, dns_name, 0, NULL, 0);
    regfree(&regex); // Free memory allocated to the pattern buffer by regcomp

    return result; 
} // validateDNSname
