# Purpose

`hblock(1)` is a shell script, available on [homebrew](https://brew.sh), that blocks ads, beacons and malware sites.
It does this by editing `/etc/hosts` and setting the IP address for such sites to 0.0.0.0. The issue is that hblock sometimes adds sites to `/etc/hosts` that are needed.

This executable fixes such issues by adding good DNS hosts to the exclusion list (`/etc/hblock/allow.list`) and removing the corresponding entry from `/etc/hosts`. It will also optionally flush the DNS cache and restart the `mDNSResponder` daemon.
