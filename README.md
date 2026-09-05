# fix-hostfiles

`hblock(1)` blocks advertising, tracking, and malware domains by mapping them to
`0.0.0.0` in `/etc/hosts`. Occasionally a domain that should remain reachable is
included. `fix-hostfiles` manages the backup/restore workflow and adds those
domains to hblock's allow list.

## Requirements

- A C11 compiler
- `hblock` for the `prep` action
- macOS for DNS-cache flushing
- Sufficient privileges for changes beneath `/etc`

## Build and test

```sh
make
make test
make release
```

The default build enables strict compiler warnings. `make test` also runs nine
isolated behavior tests. The tests redirect the hosts and hblock paths to a
temporary fixture and never modify `/etc`.

## Usage

```text
fix-hostfiles [OPTIONS] <ACTION>

Actions:
  prep                  Back up /etc/hosts and run hblock
  restore               Restore /etc/hosts from /etc/hosts-ORIG

Options that act as actions:
  -a, --add DNS_NAME    Add a domain to the allow list and unblock it
  -f, --flush           Flush the macOS DNS cache and restart mDNSResponder
  -h, --help            Display help

Modifier:
  -v, --verbose         Display detailed paths, commands, and file metadata
```

Only one action may be supplied at a time. System-file operations should
normally be run with `sudo`:

```sh
sudo fix-hostfiles prep
sudo fix-hostfiles restore
sudo fix-hostfiles --add example.domain.com
sudo fix-hostfiles --flush
sudo fix-hostfiles --verbose prep
```

Both `prep` and `restore` ask before overwriting an existing destination file.
The add operation creates `/etc/hosts.bak`, adds the DNS name only when absent,
and removes only a whitespace-delimited exact match from `/etc/hosts`.

Host-file removal uses atomic replacement: the complete revised contents are
written to a uniquely named temporary file beside the active hosts file,
flushed to disk, and assigned the original permissions and ownership. A
same-filesystem `rename(2)` then replaces the active file in one operation, so
readers see either the complete old version or the complete new version rather
than a partially rewritten file. If preparation fails before the rename, the
original remains in place and the temporary file is removed.

Verbose mode reports the relevant file type, symbolic and octal permissions,
owner, group, size, inode, link count, and modification time before and after a
filesystem action. For DNS flushing, it reports the platform and commands.

## Test configuration

`FIX_HOSTFILES_ETC_DIR` and `FIX_HOSTFILES_HBLOCK_DIR` override the production
paths. They exist to support isolated tests and development fixtures.

Generate `readme.pdf` with `make docs`, or install the executable and man page
under `/opt/homebrew` with `make install`.

The companion Bash implementation lives in the separate `fix-hosts-bash`
repository.
