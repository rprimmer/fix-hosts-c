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
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#ifdef DEBUG
#define ETC "./etc/" 
#else
#define ETC "/etc/"
#endif // DEBUG

#define HOSTS ETC "hosts"
#define HOSTS_ORIG ETC "hosts-ORIG"
#define HOSTFILES "hosts*"

#define HBLOCK "hblock"
#define ALLOWLIST ETC HBLOCK "/allow.list"
#define HBLOCK_DIR ETC "hblock"



void usage(const char *);

int updateHostsFiles(const char *, const char *, Action);

int addDnsName(const char *, const char *, const char *);

int dnsFlush(void);

#endif // FIX_HOSTS_H