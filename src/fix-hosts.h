/**
 * @file fix-hosts.h
 * @author Robert Primmer (https://github.com/rprimmer)
 * @brief Modify /etc/hosts for use with hblock(1)
 * @details hblock(1) is a shell script, available on 
 * [homebrew](https://brew.sh), that blocks ads, beacons and malware sites. It
 * does this by editing `/etc/hosts` and setting the IP address for such sites
 * to 0.0.0.0. The issue is that hblock sometimes adds sites to /etc/hosts that
 * are needed. This executable fixes such issues by adding good DNS hosts to
 * the exclusion list (`/etc/hblock/allow.list`) and removing the corresponding
 * entry from `/etc/hosts`. It will also optionally flush the DNS cache and 
 * restart the `mDNSResponder` daemon.
 * @version 1.0
 * @date 2024-03-20
 */

#ifndef FIX_HOSTS_H
#define FIX_HOSTS_H

#include "system-actions.h"
#include "actions.h"

#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
#include <libgen.h>
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
#define HBLOCK_DIR ETC HBLOCK 

/**
 * @brief Display usage
 * 
 * @param program Basename of calling program
 */
void usage(const char *program);

/**
 * @brief Modify /etc/hosts 
 * 
 * @param src Source file for action, typically hosts or hosts-ORIG
 * @param dst Destination for action, typically hosts or hosts-ORIG
 * @param action Action being performed. ACTION_PREP is action of interest.
 * @return int Return status
 */
int updateHostsFiles(const char *src, const char *dst, Action action);

/**
 * @brief Add DNS name to hblock allow list
 * 
 * @param hblock_dir Directory holding hblock allow list
 * @param dns_name DNS name to add
 * @param allow_file Name of allow list file
 * @return int Return status 
 */
int addDnsName(const char *hblock_dir, const char *dns_name, const char *allow_file);

/**
 * @brief Flush the DNS cache and restart mDNSResponder daemon 
 *
 * @return int Return status 
 */
int dnsFlush(void);

#endif // FIX_HOSTS_H