# Purpose

`hblock(1)` is a shell script, available on [homebrew](https://brew.sh), that blocks ads, beacons and malware sites. It does this by editing `/etc/hosts` and setting the IP address for such sites to 0.0.0.0. The issue is that hblock sometimes adds sites to /etc/hosts that are needed. 

This executable fixes such issues by adding good DNS hosts to the exclusion list (`/etc/hblock/allow.list`) and removing the corresponding entry from `/etc/hosts`. It will also optionally flush the DNS cache and restart the `mDNSResponder` daemon.

## Versions
There are two versions of the solution: 
* a bash shell script (`fix-hostfiles.sh`) located in `src/fix-hosts-bash`, and a
* C program (`fix-hostfiles.c`) located in `src/fix-hosts-c` that does the same.

The shell script does all that needs doing and does so in a lightweight manner. The motivation for the C version was twofold:
* To measure the performance difference between the two solutions, and
* To see how easy or difficult it would be for a C program to perform the same functions.

My a priori predictions are:
* the executable will be substantially less performant, mainly due to the overhead of instantiating the program — and the programs it would in turn spawn — and 
* that the C program would be harder to concoct since I don't think that there are library functions to perform all the stuff I get for free in shell land (e.g. `sed(1)`, `grep(1)` and `dscacheutil(1)`), but I knew I'd learn something along the way.

### Design Specs and man pages
There are design specs for each solution, though I never went back to update them after the coding was done, so they're inexact replicas of the actual code. 

I wrote the C design spec after completing the bash script, so this spec is a bit more updated. 

The associated man pages (the man pages for each solution have minor but meaningful differences) do a better job of explaining the final products. 

## TODO
- [x] Write the design spec for the C implementation
- [ ] Update the bash design spec
- [x] Create a separate MD file for hblock allow list with hard link between both project dirs 
