// system-actions.h
// Collection of common system functions 

#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int booleanQuery(const char *);

void handleError(const char *);

int fileExists(const char *);

int copyFile(const char *, const char *);

int copyFile2(const char *, const char *); 

int lsFiles(const char *, const char *);

int fileInfo(const char *); 
