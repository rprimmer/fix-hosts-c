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

### AI Recommendations
Below are recommendations made by [Gemini](#gemini-suggestions) and [ChatGPT](#chatgpt-suggestions). 

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
- [ ] Compare costs/benefits of `system(3)` to `exec*` calls
- [ ] Update man page to the new format I discovered (if I can ever locate it again :))
- [ ] Should `handleError()` include `__LINE__` and calling function name?
- [ ] Update bash script with any updates from C project 
- [ ] Create the option to copy updates made to `hosts{,.allow}` files to other systems (mac or linux) somehow (shared dropbox folder?)

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

## Variables 
``` C
const char *const HOSTS = "/etc/hosts";
const char *const HBLOCK = "/etc/hblock";
const char *const ALLOWS = "allow.list";
enum action_t {
    ACTION_PREP,
    ACTION_RESTORE,
    ACTION_ADD,
    ACTION_FLUSH,
    ACTION_INVALID, 
};
```
`enum action_t action = ACTION_INVALID;`

`char *DNS_NAME;`

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

## Gemini Suggestions
 I'm ready to help you identify alternatives to `system` calls in the provided Bash script. Here's a breakdown of the functions and their potential replacements:

1. `copyHostsFile()`:
- Current Actions:
  - Checks for `hosts` file existence (`test -f` or `stat`)
  - Copies `hosts` to `hosts-ORIG` (`cp`)
  - Runs `hblock` (external command)
- Alternatives:
  - Use `open`, `read`, and `write` system calls for file operations.
  - Explore directly calling `hblock`'s functionality if available (e.g., through a library).
2. `restoreHostsFile()`:
- Current Actions:
  - Checks for `hosts-ORIG` existence (`test -f` or `stat`)
  - Copies `hosts-ORIG` to `hosts` (`cp`)
- Alternatives:
  - Use `open`, `read`, and `write` system calls for file operations.
3. `addDNSname()`:
- Current Actions:
  - Validates DNS name format (`grep` for regular expression matching)
  - Creates `/etc/hblock` directory if needed (`mkdir`)
  - Appends DNS name to `allow.list` (`echo` and redirection)
  - Removes DNS name from `/etc/hosts` (`sed`)
- Alternatives:
  - Implement regular expression matching using functions like `regcomp` and `regexec`.
  - Use `open`, `write`, and `ftruncate` system calls for file operations.
  - Explore string manipulation functions like `strtok` or `strstr` for finding and removing lines in `hosts`.
4. `flushDNScache()`:
- Current Actions:
  - Checks for macOS (`uname`)
  - Flushes DNS cache (`dscacheutil -flushcache`)
  - Restarts `mDNSResponder` service (`sudo killall -HUP`)
- Alternatives:
  - Use macOS-specific APIs for flushing DNS cache (`resolv.h` header).
  - Consider using `launchctl` or `Service Management` framework for service management.
5. `main()`:
- Current Actions:
  - Checks for `hblock` availability (`command -v`)
  - Processes arguments (`getopts`)
  - Calls appropriate functions based on actions
- Alternatives:
  - No direct system call alternatives for argument parsing or function calls.

### Remember:
- Adapt system calls and library functions based on your specific operating system and environment.
- Handle errors and edge cases appropriately when using lower-level system calls.
- Consider performance implications when choosing between system calls and higher-level tools like `sed` or `grep`.

## ChatGPT Suggestions
Certainly! Replacing Bash script functionalities with direct system calls or library functions in C is generally more efficient and secure than using the `system()` function. Here's how you can replace some common Bash functionalities:

1. File and Directory Operations:
   
   - Copying Files: Use `fopen()`, `fread()`, and `fwrite()` for reading from one file and writing to another.
   - Listing Files: Use `opendir()`, `readdir()`, and `closedir()` from `dirent.h` for directory operations.
   - Checking File Existence: Use `stat()` from `sys/stat.h`.

2. Executing External Programs:
   
   - Replacing `system()` Calls: Use `fork()` to create a new process, then `exec()` (like `execl()`, `execv()`, etc.) to run a new program within that process.
   - Handling Pipes and Redirection: Use `pipe()`, `dup()`, or `dup2()` for creating pipes and redirecting input/output.

3. Regular Expressions:
   
   - Validating DNS Names: Use regular expression functions from `regex.h` (e.g., `regcomp()`, `regexec()`, etc.).

4. User Input and Output:
   
   - Print Messages: Use `printf()` or `puts()` for standard output and `fprintf(stderr, ...)` for standard error.
   - Read User Input: Use `fgets()` or `scanf()`.

5. String Handling and Manipulation:
   
   - String Operations: Use functions from `string.h` like `strcpy()`, `strcat()`, `strcmp()`, etc.

6. Process Management:
   
   - Flush DNS Cache and Restart Services (macOS-specific): This typically requires executing system-specific commands, which is tricky without `system()`. However, you can use `fork()` and `exec()` family functions to run these commands. Note that this is OS-specific and might require root privileges.

7. Memory Allocation:
   
   - Dynamic Memory: Use `malloc()`, `calloc()`, `realloc()`, and `free()`.

8. Argument Parsing:
   
   - Command-Line Arguments: Handled in `main()` with `argc` and `argv`. Use loops and string comparison functions for parsing.

9. Networking:
   
   - DNS and Network Operations: You might need to use specific networking libraries or system calls, depending on what you're trying to achieve.

10. Error Handling*
    
    - Custom Error Handling: Use return values and `errno` for error handling. Custom error messages can be outputted using `perror()` or `fprintf()`.

Here's an example of how you might replace a file copy operation in C without using `system()`:

```c
#include <stdio.h>
#include <stdlib.h>

void copyFile(const char *src, const char *dest) {
    FILE *source, *destination;
    char buffer[1024];
    size_t bytesRead;

    source = fopen(src, "rb");
    if (source == NULL) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        perror("Error opening destination file");
        exit(EXIT_FAILURE);
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytesRead, destination);
    }

    fclose(source);
    fclose(destination);
}
```

This function reads from a source file and writes to a destination file, effectively copying it. You would use similar approaches to replace other Bash script functionalities with equivalent C code. Remember that direct system calls and library functions offer more control but also require careful handling of resources and errors.
