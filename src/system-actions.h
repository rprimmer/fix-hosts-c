// system-actions.h
// Collection of common system functions

#ifndef SYSTEM_ACTIONS_H
#define SYSTEM_ACTIONS_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
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

int booleanQuery(const char *);

void handleError(const char *, ...);

int fileExists(const char *);

int copyFile(const char *, const char *);

int copyFile2(const char *, const char *); 

int lsFiles(const char *, const char *);

int fileInfo(const char *);

int checkProcess(const char *);

int displayProcess(const char *);

int validateDNSname(const char *d); 

#endif /* SYSTEM_ACTIONS_H */