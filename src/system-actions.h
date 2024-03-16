/**
 * @file systems-actions.h
 * @author Robert Primmer (https://github.com/rprimmer)
 * @brief Common functions and system actions
 * @version 1.0
 * @date 2024-03-16
 */

#ifndef SYSTEM_ACTIONS_H
#define SYSTEM_ACTIONS_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// NOTE: In this macro, ##__VA_ARGS__ is a GNU extension that still works if __VA_ARGS__ is empty, 
// which supports calling HANDLE_ERROR with just a string or with additional format arguments.
// Modern compilers support this so I didn't want to clutter the code with a bunch of 
// #ifdef __GNUC__ conditionals just for the sake of some ancient compiler from a time long ago...
#define HANDLE_ERROR(fmt, ...) handleError("%s, %d: " fmt, basename(__FILE__), __LINE__, ##__VA_ARGS__)


/**
 * @brief Common error handling routine 
 * 
 * @param message Message to be displayed to user
 * @param ... Optional parameters can be provided (va_list)
 */
void handleError(const char *message, ...);

/**
 * @brief Query user for yes or no
 * 
 * @param prompt Message to be displayed to user
 * @return int Return true if user entered y or Y
 */
int booleanQuery(const char *prompt); 

/**
 * @brief Check for file existence 
 * 
 * @param filename File to check
 * @return int Return true of file exists
 */
int fileExists(const char *filename);

/**
 * @brief Make a copy of a file. Uses fread(3) & fwrite(3)
 * 
 * @param src File to be copied
 * @param dest Filename of copy
 * @return int Return status 
 */
int copyFile(const char *src, const char *dest);

/*
 * @brief Make a copy of a file. Uses read(2) & write(2)
 *
 * @param src File to be copied
 * @param dest Filename of copy
 * @return int Return status
 */
int copyFile2(const char *src, const char *dest);

/**
 * @brief List files in a directory
 * 
 * @param dirname Directory housing files
 * @param files Files to list
 * @return int Return status 
 */
int lsFiles(const char *dirname, const char *files);

/**
 * @brief Display information about a file
 * 
 * @param filepath File to stat
 * @return int Return status 
 */
int fileInfo(const char *filepath);

/**
 * @brief Check if a process is currently running
 * 
 * @param process_name Process to look for
 * @return int Return status 
 */
int checkProcess(const char *process_name);

/**
 * @brief Display info on a running process
 *
 * @param process_name Process to look for
 * @return int Return status
 */
int displayProcess(const char *process_name);

/**
 * @brief DNS name must start & end with a letter or a number and can only contain letters, numbers, and hyphens.
 *
 * @param dns_name DNS name to check
 * @return int Return status 
 */
int validateDNSname(const char *dns_name);

#endif /* SYSTEM_ACTIONS_H */