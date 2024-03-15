# Design Specification `fix-hostfiles` C Program

## Design Considerations

### Consistency

Where possible, will try to maintain the same structure in the C program as exists in the bash script, such as function and variable names.

### Globals

Globals in bash are the default. In C we may be able to dispense with these, though `#define` statements will still be helpful for the preprocessor.

### Use of system(3)

Some of the functions in the bash script (e.g., reading and writing) can be accomplished in C with library functions and/or system calls, other cannot. For those cases, will experiment with different methods.

* system(3)
* execve(2)
* fork(2)

## Binary executable

* Binary executable located in `~/bin` called `fix-hostfile`

## Arguments

* **restore** : restores original hosts file, displays output
* **prep** : creates copy of original hosts file, displays output, calls `hblock(1)`
* DNS name to add with `-a` switch

## Switches

* `-a, --add`   : add DNS entry to allow.list, delete it from hosts
* `-f, --flush` : flush DNS cache and restart the `mDNSResponder` service
* `-h, --help`  : Display usage

`main()`

* Handle switches
* Handle arguments
* Handle actions

`void usage(const char *program)`

* Display help to user

`int updateHostsFiles(const char *src, const char *dst, Action action)`

* Modify /etc/hosts
* PREP is essentially cp hosts{,-ORIG}
* RESTORE is the inverse 
* if `(action == ACTION_PREP)`, run `hblock(1)`

`int addDnsName(const char *hblock_dir, const char *dns_name, const char *allow_file)`

* Validate DNS name.
* Add valid DNS name to hblock exception list.
* Run hblock(1), which achieves the same result (i.e., removing the good DNS name from /etc/hosts) without the need for sed(1).

`int dnsFlush(void)`

* Verify running on macOS (Darwin). 
* Flush DNS cache. 
* Restart mDNSResponder daemon.
* if `action = ACTION_PREP`, run `hblock(1)`.

## Results

At the start I believed that this program should have inferior performance to the bash script (`fix-hostfiles.sh`). First, there's the cost of instantiating a new process image. Second, part of the functions in the code required `system(3)`.

Therefore, the primary goal of this project was to see how well these same functions could be performed in a C program. While bash scripts are enormously useful, C is nicer to code in -- at least for me.

In practice I didn't find the performance difference to be meaningful. It turned out that instantiation isn't as bad as it is for GUI programs, which are often surprisingly slow on macOS, despite plenty of free memory and CPU. Plus the program itself is small enough that runtime after instantiation isn't material to a human user.

## Lessons

1. **Separate Folders**: It's better to have separate folders for each project as VSC does better with this. 

    * Initially I tried having both the C program and bash executable in the same directory, but this caused complications with both VSC and git.

2. **basename**: There's a limitation in `basename(3)`; it reuses the same memory address. This messed up subsequent use of the library call within the same translation unit.
    * In `main.c` I had used `char *program = basename[argv[0])`, but this meant that the address pointed to was overwritten whenever I called `basename()` again. E.g., when calling `fprintf(stderr, "%s, %d: argc: %d, optind: %d\n", basename(__FILE__), __LINE__, argc, optind);` subsequent, the address was overwritten and now the original value for `program` was lost.
    * From the manpage: The `basename()` function returns a pointer to internal storage space allocated on the first call that will be overwritten by subsequent calls. `basename_r()` is therefore preferred for threaded applications.
        * In reality, `basename_r()` is preferred whenever you want to persist the returned string.
    * The fix was: `char program[PATH_MAX]; basename_r(argv[0], program);`
        * `PATH_MAX` requires `#include <limits.h>`
    * Interestingly, the memory address used by `basename()` does *not* persist over translation units. Therefore, I'm free to use `basename()` for the `fprintf()` calls I use for debugging and it works as expected; i.e., each TU gets its own unique memory address for calls to `basename()`.

3. **Rewriting**: The saying "it's not the writing but the rewriting" is true for coding as well.
    * I was surprised to discover things that I missed when creating the bash version of this. In retrospect, these changes should have been self evident.
    * For example, I had two functions (`copyHostsFile` and `restoreHostsFile`) in the bash script. Only when writing this is C did it become plainly obvious that these two functions should be in a single function (`updateHostsFiles`).

4. **Manpage**: I tried to write the manpage in markdown and then use `pandoc(1)` to create the manpage. This *mostly* worked, but introduced limitations in the output that ultimately proved not worth it. Having an existing manpage as a template and using that became easy enough without sacrificing control of the resulting output.

5. **Doxygen**: It's good to wait until the code is fully completed before adding doxygen API comments.
    * I'm ambivalent about  whether I prefer these doc comments in the .c file or the corresponding .h.
    * I like the cleaner look of the c files sans api doc comments, but if these are in the .h file, the reader has to bounce back to the header file to see the api doc. 
    * Also, for the doxygen VSC extension to work in the .h file requires that you explicitly name the variables in the header file; e.g., `void usage(const char *program)` instead of just `void usage(const char *)`.

## TODO

* [x] Update makefile -- one for bash script, one for C executable
* [x] Typedef within a typedef?
* [x] Create SERROR?
* [x] Move processArguments code to main()?
* [x] Consider `perror()` in `handleError()`.
* [x] Consider globals (e.g., `const char* ETC = "/etc";`) in place of hardcoded strings such as "etc". UPDATE: went with `#define`.
* [x] Move system functions to separate files (system-actions.{c,h}).
* [x] Research use of long switches (e.g., `--help`).
* [x] Right now the switches are mutually exclusive. Should -fa be allowed?
* [x] Modify {copy,restore}HostFiles() to be a single function `updateHostFiles(action)` with prep|restore as parameter?
* [x] Compare costs/benefits of `system(3)` to `exec*` calls.
* [x] Update man page to the new format I discovered (if I can ever locate it again :)).
* [x] Should `handleError()` include `__LINE__` and calling function name?
