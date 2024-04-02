/**
 * @file system-actions.h
 * @author Robert Primmer (https://github.com/rprimmer)
 * @brief Common functions and system actions.
 * @version 1.3
 * @date 2024-04-02
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// ##__VA_ARGS__ is a GNU extension that still works if __VA_ARGS__ is empty, 
// which supports calling the macro with just a string or with additional format arguments.
// Modern compilers support this so I didn't want to clutter the code with a bunch of 
// #ifdef __GNUC__ conditionals just for the sake of some ancient compiler from long long ago.
// __func__ was introduced in C99. 
#define HANDLE_ERROR(fmt, ...) handleError(true, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define REPORT_ERROR(fmt, ...) handleError(false, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Common error handling routine
 * 
 * @param fatal If true, exit program, else returns to the caller.
 * @param file C filename (translation unit) of caller.
 * @param func Function name of caller.
 * @param line Line number in translation unit.
 * @param fmt Optional parameters can be provided (va_list).
 */
void handleError(bool fatal, char *file, const char *func, int line, const char *fmt, ...);

/**
 * @brief Query user for yes or no.
 * 
 * @param prompt Message to be displayed to user.
 * @return int Return true if user entered y or Y.
 */
int booleanQuery(const char *prompt); 

/**
 * @brief Check for file existence.
 * 
 * @param filename File to check.
 * @return int Return true if file exists.
 */
int fileExists(const char *filename);

/**
 * @brief Make a copy of a file. Uses libC functions.
 * 
 * @param src File to be copied.
 * @param dest Filename of copy.
 * @return int Return status.
 */
int copyFile(const char *src, const char *dest);

/**
 * @brief Make a copy of a file. Uses system calls.
 *
 * @param src File to be copied.
 * @param dest Filename of copy.
 * @return int Return status.
 */
int copyFile2(const char *src, const char *dest);

/**
 * @brief List files in a directory.
 * 
 * @param dirname Directory housing files.
 * @param files Files to list.
 * @return int Return status. 
 */
int lsFiles(const char *dirname, const char *files);

/**
 * @brief Display information about a file.
 * 
 * @param filepath File to stat(2).
 * @return int Return status.
 */
int fileInfo(const char *filepath);

/**
 * @brief Check if a process is currently running. 
 * 
 * @param process_name Process to look for.
 * @param display_pids Optionally display PIDs of running process.
 * @return int Return status.
 */
int checkProcess(const char *process_name, bool display_pids);

/**
 * @brief Display info on a running process.
 *
 * @param process_name Process to look for.
 * @return int Return status.
 */
int displayProcess(const char *process_name);

/**
 * @brief DNS name must start & end with a letter or a number and can only contain letters, numbers, and hyphens.
 *
 * @param dns_name DNS name to check.
 * @return int Return status.
 */
int validateDNSname(const char *dns_name);

#endif /* SYSTEM_ACTIONS_H */