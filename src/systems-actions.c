#include "system-actions.h"

void handleError(const char *msg) {
    if (errno) 
        perror(msg); 
     else 
        fprintf(stderr, "Error: %s\n", msg);
    
    exit(EXIT_FAILURE);
} // handleError()

int booleanQuery(const char *prompt) {
    char response[10];

    printf("%s ", prompt);

    if (fgets(response, sizeof(response), stdin) == NULL) 
        handleError("failed to read user response");

    // Empty newline is interpreted as non-yes answer 
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
        handleError("error opening source file");

    destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        handleError("error opening destination file");
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
        handleError("failed to open source file");

    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) 
        handleError("failed to create destination file"); 

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1) 
            handleError("write error");
    }

    if (bytes_read == -1) 
        handleError("read error");

    close(source_fd);
    close(dest_fd);

    printf("File copied successfully!\n");

    return EXIT_SUCCESS;
} // copyFile2()

int lsFiles(const char *dirname, const char *files) { 
#ifdef DEBUG
    fprintf(stderr, "In function: lsFiles\n");
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

    closedir(dir);

    return EXIT_SUCCESS;  
    
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
