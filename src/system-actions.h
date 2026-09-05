/**
 * @file system-actions.h
 * @brief Reusable filesystem, process, validation, and reporting helpers.
 */

#ifndef SYSTEM_ACTIONS_H
#define SYSTEM_ACTIONS_H

#include <stdbool.h>

/** Report an error with the calling file, function, and line number. */
#define REPORT_ERROR(...) reportError(__FILE__, __func__, __LINE__, __VA_ARGS__)

/**
 * Report a formatted error with its source location.
 *
 * @param file Translation unit supplied by REPORT_ERROR.
 * @param function Calling function supplied by REPORT_ERROR.
 * @param line Source line supplied by REPORT_ERROR.
 * @param format printf-compatible message format.
 */
void reportError(const char *file, const char *function, int line,
                 const char *format, ...);

/**
 * Repeatedly prompt for a yes-or-no response.
 *
 * @param prompt Question displayed before the input prompt.
 * @return true for yes; false for no or end-of-file.
 */
bool confirm(const char *prompt);

/**
 * Test whether a filesystem path exists.
 *
 * @param path Path to inspect.
 * @return true when access(path, F_OK) succeeds.
 */
bool fileExists(const char *path);

/**
 * Copy a regular file and preserve its permission bits.
 *
 * @param source_path Existing file to copy.
 * @param destination_path Destination to create or overwrite.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on any I/O error.
 */
int copyFile(const char *source_path, const char *destination_path);

/**
 * List directory entries matching a shell-style pattern.
 *
 * @param directory_path Directory to scan.
 * @param pattern Pattern interpreted by fnmatch(3).
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on directory or stat errors.
 */
int listFiles(const char *directory_path, const char *pattern);

/**
 * Display detailed metadata for one filesystem object.
 *
 * The report includes type, permissions, ownership, size, inode, link count,
 * and last-modification time. Symbolic links are inspected without following
 * their targets.
 *
 * @param path Filesystem object to inspect.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE when metadata is unavailable.
 */
int fileInfo(const char *path);

/**
 * Create a directory if it does not already exist.
 *
 * @param path Directory to validate or create.
 * @return EXIT_SUCCESS when a directory exists at path; otherwise EXIT_FAILURE.
 */
int ensureDirectory(const char *path);

/**
 * Execute an argument vector without invoking a shell.
 *
 * The function forks, calls execvp(3) in the child, and waits for that child.
 *
 * @param arguments NULL-terminated argv array whose first element is a command.
 * @return The child's exit status, or EXIT_FAILURE when it cannot be executed.
 */
int runCommand(char *const arguments[]);

/**
 * Validate a fully qualified DNS name.
 *
 * Labels may contain letters, numbers, and interior hyphens. The final label
 * must contain at least two letters.
 *
 * @param dns_name Candidate DNS name.
 * @return true when the complete string matches the accepted DNS syntax.
 */
bool isValidDnsName(const char *dns_name);

/**
 * Search a text file for an exact line.
 *
 * Line endings are removed before comparison.
 *
 * @param path File to search.
 * @param expected_line Complete line to locate, without a newline.
 * @return true when the line is found; false otherwise.
 */
bool fileContainsLine(const char *path, const char *expected_line);

/**
 * Append one newline-terminated line to a file.
 *
 * @param path File to create or append to.
 * @param line Text to append without a terminating newline.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on an I/O error.
 */
int appendLine(const char *path, const char *line);

#endif
