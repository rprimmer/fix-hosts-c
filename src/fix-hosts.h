/**
 * @file fix-hosts.h
 * @brief Hosts-file paths and application operations.
 */

#ifndef FIX_HOSTS_H
#define FIX_HOSTS_H

#include "actions.h"

#include <limits.h>
#include <stdio.h>

#ifndef PATH_MAX
/** Fallback maximum path length for platforms that do not define PATH_MAX. */
#define PATH_MAX 4096
#endif

/** Filesystem paths used by hosts-file operations. */
typedef struct {
    /** Directory containing the system hosts files. */
    char etc_dir[PATH_MAX];
    /** Active hosts file. */
    char hosts_file[PATH_MAX];
    /** Backup used by prep and restore. */
    char original_hosts_file[PATH_MAX];
    /** Directory containing hblock configuration. */
    char hblock_dir[PATH_MAX];
    /** hblock DNS allow list. */
    char allow_file[PATH_MAX];
} Paths;

/**
 * Print command help.
 *
 * @param program Basename used to invoke the executable.
 * @param stream Destination stream, normally stdout or stderr.
 */
void usage(const char *program, FILE *stream);

/**
 * Load all filesystem paths used by the application.
 *
 * Production paths are derived from /etc. The FIX_HOSTFILES_ETC_DIR and
 * FIX_HOSTFILES_HBLOCK_DIR environment variables provide explicit overrides
 * for isolated tests and development fixtures.
 *
 * @param paths Structure populated with validated, absolute or relative paths.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE if a path exceeds PATH_MAX.
 */
int loadPaths(Paths *paths);

/**
 * Back up or restore the system hosts file.
 *
 * Existing destination files require interactive confirmation. ACTION_PREP
 * runs hblock after creating the backup; ACTION_RESTORE copies the backup over
 * the active hosts file.
 *
 * @param paths Filesystem paths used by the operation.
 * @param action ACTION_PREP or ACTION_RESTORE.
 * @param verbose Whether to display detailed before-and-after file metadata.
 * @return EXIT_SUCCESS on success or user cancellation; EXIT_FAILURE on error.
 */
int updateHostsFiles(const Paths *paths, Action action, bool verbose);

/**
 * Add a DNS name to hblock's allow list and unblock it.
 *
 * The allow-list update is idempotent. Afterward, matching hosts-file lines
 * are removed through removeHostEntry().
 *
 * @param paths Filesystem paths used by the operation.
 * @param dns_name DNS name to add and unblock.
 * @param verbose Whether to display detailed before-and-after file metadata.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on validation or I/O error.
 */
int addDnsName(const Paths *paths, const char *dns_name, bool verbose);

/**
 * Atomically remove hosts-file lines containing an exact DNS token.
 *
 * The original file is first copied to a .bak file. Retained lines are written
 * to a same-directory temporary file, synchronized, assigned the original
 * permissions and ownership, and atomically renamed over the hosts file.
 *
 * @param hosts_path Path to the active hosts file.
 * @param dns_name Validated DNS token to remove.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on any filesystem error.
 */
int removeHostEntry(const char *hosts_path, const char *dns_name);

/**
 * Flush the macOS DNS cache and restart mDNSResponder.
 *
 * Commands are executed directly without invoking a shell. The function also
 * verifies that mDNSResponder is present after the restart.
 *
 * @param verbose Whether to display platform and command details.
 * @return EXIT_SUCCESS when all commands succeed and the daemon is found;
 *         EXIT_FAILURE on unsupported platforms or command failure.
 */
int flushDnsCache(bool verbose);

#endif
