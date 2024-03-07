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

// const char *const ETC = "./etc/";
// const char *const HOSTS = "./etc/hosts";
// const char *const HOSTS_ORIG = "./etc/hosts-ORIG";
// const char *const HOSTFILES = "hosts*";
// const char *const HBLOCK = "hblock";
// const char *const ALLOWLIST = "/etc/hblock/allow.list";

#define  ETC "./etc/"
#define  HOSTS "./etc/hosts"
#define  HOSTS_ORIG "./etc/hosts-ORIG"
#define  HOSTFILES "hosts*"
#define  HBLOCK "hblock"
#define  ALLOWLIST "/etc/hblock/allow.list"

void usage(const char *);

int updateHostsFiles(const char *, const char *, Action);

int copyHostsFiles(void);

int restoreHostsFile(void);

int addDnsName(const char *);

int dnsFlush(void);

#endif // FIX_HOSTS_H