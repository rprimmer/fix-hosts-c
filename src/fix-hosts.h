// fix-hosts.h

#pragma once

#include "system-actions.h"
#include <dirent.h>
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

void usage(const char *);

int copyHostsFiles(void);

int restoreHostsFile(void);

int addDnsName(const char *);

int dnsFlush(void);
