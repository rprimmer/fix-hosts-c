// fix-hosts.h

#pragma once

void usage(char *);

void handleError(char *);

int prepHostsFile(void);

int copyHostsFiles(void);

int restoreHostsFile(void);

int addDnsName(char *);

int dnsFlush(void);
