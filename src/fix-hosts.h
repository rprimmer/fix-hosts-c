// fix-hosts.h

#ifndef FIX_HOSTS_H
#define FIX_HOSTS_H

#include "system-actions.h"
#include "actions.h"

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

#define ETC "./etc/"
#define HOSTS ETC "hosts"
#define HOSTS_ORIG ETC "hosts-ORIG"
#define ALLOWLIST ETC "hblock/allow.list"
#define HOSTFILES "hosts*"
#define HBLOCK "hblock"

void usage(const char *);

int updateHostsFiles(const char *, const char *, Action);

int addDnsName(const char *);

int dnsFlush(void);

#endif // FIX_HOSTS_H