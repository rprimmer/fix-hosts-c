# Design Specification -- C Program

## Design Considerations
### Consistency
Where possible, will try to maintain the same structure in the C program as exists in the bash script, such as function and variable names.

### Globals
Globals in Bash are the default. In C we may be able to dispense with these. 

### Use of system(3)
Some of the functions in the bash script (e.g., reading and writing) can be accomplished in C with library functions or system calls, other cannot. For those cases, will experiment with different methods.

* system(3)
* execve(2)
* fork(2)


## TODO
- [x] Update makefile -- one for bash script, one for C executable 
- [x] Typedef withing a typedef? 
- [x] Create SERROR? 
- [x] Move processArguments code to main()?
- [x] Consider `perror()` in `handleError()`
- [x] Consider globals (e.g., `const char* ETC = "/etc";`) in place of hardcoded strings such as "etc"
- [x] Move system functions to separate files (system-actions.{c,h}) 
- [x] Research use of long switches (e.g., `--help`)
- [x] Right now the switches are mutually exclusive. Should -fa be allowed?
- [x] Modify {copy,restore}HostFiles() to be a single function `updateHostFiles(action)` with prep|restore as parameter? 
- [x] Compare costs/benefits of `system(3)` to `exec*` calls
- [ ] Update man page to the new format I discovered (if I can ever locate it again :))
- [x] Should `handleError()` include `__LINE__` and calling function name?
- [ ] Update bash script with updates from C project 
- [ ] Create option to copy updates made to `hosts{,.allow}` files to other systems (mac or linux) somehow (shared dropbox folder?)

## Binary executable
* Binary executable located in `~/bin` called `fix-hostfile-c`

## Arguments
* restore : restores original hosts file, displays output
* prep : creates copy of original hosts file, displays output, calls hblock(1)
* DNS name to add with -a switch 

## Switches
* -a  : add IP entry to allow.list, delete it from hosts
* -f  : flush DNS cache and restart the `mDNSResponder` service
* -h  : Display usage

## main()
Parse args and switches, call functions
* DNS_FLUSH = FALSE
* ADD_DNS = FALSE 
* IF malformed input, call usage(), return EXIT_FAILURE
* IF -h | —help switch, call usage(), return EXIT_FAILURE
* IF -f | —flush switch, set DNS_FLUSH = TRUE
* IF -a | —add switch, set ADD_DNS = TRUE
* IF arg1 == restore, call restore_hosts_file()
* ELIF arg1 == prep, call prep_hosts_file()
* IF ADD_DNS = TRUE, call add_dns_name()
* IF DNS_FLUSH = TRUE, call dns_flush()
* return EXIT_SUCCESS; 

## usage()
* printf(stderr, "%s\n", "usage: basename $0 <ahf> <prep | restore> <DNS entry>");
* exit 1

## copyHostsFiles()
* echo “Existing hosts files”
* sudo ls -las /etc/hosts*
* Check for existence of files: hosts and hosts-ORIG.
  * IF hosts file doesn’t exist, display error and exit_failure.
* sudo cp hosts{,-ORIG}
* echo “Running hblock to create new hosts file…”
* hblock
* echo “Updated hosts files”
* sudo ls -las hosts*
* return 

## restore_hosts_file()
* cd /etc
* echo “Existing hosts files”
* sudo ls -las hosts*
* Check for existence of files hosts and hosts-ORIG.
  * IF either file doesn’t exist, display error, return 1.
* Check for existence of hosts-HBLOCK. IF it exists, 
  * Warn that this action will delete this file.
  * Query user to continue or exit.  
  * IF continue,
    * sudo mv hosts{,-HBLOCK}
    * sudo cp hosts{-ORIG,}
    * echo “Updated hosts files”
      * sudo ls -las hosts*
* return

## add-dns-name(name)
* char* host = name; 
* if (name == NULL) usage(), return 1;
* echo “Adding <name> to /etc/hblock/allow.list”
* cd /etc/hblock
* Check if <name> already exists in allow.list. IF present
  * echo “Entry <name> already exists in allow.list”
* ELSE
  * cat >> allow.list <name>
  * echo “Updated allow.host”
* cat allow.list
* cd /etc
* Verify <name> is in hosts file.  IF present,
  * echo “Removing <name> from /etc/hosts
  * sed command to delete <name> from hosts file
* ELSE
  * echo “Entry <name> is not present in hosts file”
* echo “Actions completed”
* return  

## dns-flush()
* echo “Flushing DNS cache…”
* sudo dscacheutil -flushcache
* sleep
* echo “Restarting  the mDNSResponder service…”
* sudo killall -HUP mDNSResponder
* sleep
* echo “New mDNSResponder PID” 
  * Display mDNSResponder PID
* return

## Results
I knew going in that this program was done far better as a bash script. I also knew that the performance of the script would be superior to instantiating a binary executable to do the same functions. 

Therefore, the primary goals of this project were:
* To learn, and
* To test the performance difference between the two solutions.

### Lessons 
1. It's better to have separate folders for each project as VSC does better with this.

### Performance Tests
No surprise, the bash script performed better. Here's the average times.
