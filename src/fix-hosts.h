#pragma once

void usage(void);

int prepHostsFile(void);

int copyHostsFiles(void); 

int restoreHostsFile(void);

int addDnsName(char* name);

int dnsFlush(void);
