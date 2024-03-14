#ifndef ACTIONS_H
#define ACTIONS_H

/**
 * @brief Actions a user can perform, based on switches and arguments
 * 
 */
typedef enum {
    ACTION_PREP,
    ACTION_RESTORE,
    ACTION_ADD,
    ACTION_FLUSH,
    ACTION_INVALID
} Action;

#endif /* ACTIONS_H */