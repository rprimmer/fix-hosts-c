/**
 * @file actions.h
 * @brief Command actions and parsed command-line state.
 */

#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdbool.h>

/** Actions supported by the command-line interface. */
typedef enum {
    /** No action was selected, or parsing failed. */
    ACTION_INVALID = 0,
    /** Back up the active hosts file and run hblock. */
    ACTION_PREP,
    /** Restore the active hosts file from its original backup. */
    ACTION_RESTORE,
    /** Add a DNS name to the hblock allow list and unblock it. */
    ACTION_ADD,
    /** Flush the macOS DNS cache and restart mDNSResponder. */
    ACTION_FLUSH
} Action;

/** Parsed command-line request. */
typedef struct {
    /** Operation to perform. */
    Action action;
    /** Domain supplied to ACTION_ADD; otherwise NULL. */
    const char *dns_name;
    /** Whether to display detailed paths, commands, and file metadata. */
    bool verbose;
} Command;

#endif
