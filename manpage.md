% fix-hostfiles(1)
% Robert Primmer
% March 2024

## NAME
fix-hostfiles - Manage /etc/hosts for hblock(1) 

## SYNOPSIS
**fix-hostfiles** \[OPTION\] \<ACTION\>

## DESCRIPTION
hblock(1) is a shell script (available on homebrew) that blocks ads, beacons and malware sites. It does this by editing /etc/hosts and setting the IP address for such sites to 0.0.0.0. The issue is that hblock sometimes adds sites to /etc/hosts that are needed. This program fixes such issues by adding good DNS hosts to the exclusion list (/etc/hblock/allow.list) and removing the corresponding entry from /etc/hosts. It will also optionally flush the DNS cache and restart the mDNSResponder daemon.

## OPTIONS
**-h, -\-help**:
    Display this help message and exit

**-r, -\-flush**:
    Flush DNS cache and restart mDNSResponder daemon

**-a, -\-add \<DNS name\>**:
    Add a DNS entry to allow list and remove from /etc/hosts

## ACTIONS
**prep**:
    Backup hosts file and run hblock to create a new hosts file.

**restore**:
    Reinstate original hosts file.

## RESTRICTIONS
The \<flush\> action is for macOS only.

Requires privileged actions. User must know sudo(1) password.

## NOTES
When adding a DNS entry (-a, -\-add) or flushing the cache (-f, -\-flush), the arguments \<prep | restore\> are not required.

## EXAMPLES
sudo fix-hostfiles prep 

sudo fix-hostfiles restore

sudo fix-hostfiles -a example.domain.com

sudo fix-hostfiles -f