#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void handleError(const char *msg) {
    if (errno != 0) 
        perror(msg); 
     else 
        fprintf(stderr, "Error: %s\n", msg);
    
    exit(EXIT_FAILURE);
}

int booleanQuery(const char *prompt) {
    char response[10];

    printf("%s ", prompt);

    // Read the entire input line, including any newline
    if (fgets(response, sizeof(response), stdin) == NULL) 
        handleError("Failed to read user response");

    // Empty newline is interpreted as non-yes answer 
    if (response[0] == '\n')     
        return 0;

    // Otherwise, check the first non-whitespace character
    return (response[0] == 'y' || response[0] == 'Y');
}

int fileExists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void copyFile(const char *src, const char *dest) {
    FILE *source, *destination;
    char buffer[1024];
    size_t bytesRead;

    source = fopen(src, "rb");
    if (source == NULL) {
        handleError("Error opening source file");
    }

    destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        handleError("Error opening destination file");
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytesRead, destination);
    }

    fclose(source);
    fclose(destination);
}

int fileCopy2(const char *src, const char *dest) {
    int source_fd, dest_fd;
    char buffer[4096]; 
    ssize_t bytes_read, bytes_written;

    // Open source file for reading
    source_fd = open(src, O_RDONLY);
    if (source_fd == -1) {
        handleError("Failed to open source file");
    }

    // Create destination file with appropriate permissions
    // (You may need to adjust the permissions based on your requirements)
    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        handleError("Failed to create destination file");
    }

    // Read from source and write to destination in a loop
    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            handleError("Write error");
        }
    }

    if (bytes_read == -1) {
        handleError("Read error");
    }

    // Close file descriptors
    close(source_fd);
    close(dest_fd);

    printf("File copied successfully!\n");

    return EXIT_SUCCESS;
}