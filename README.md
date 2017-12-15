Demonstrate a 'bug' (quirk?) using SEEK_DATA/SEEK_HOLE on ZFS

... you almost certainly aren't interested in this code if you didn't
come here specifically looking for it ...

The code is a but rough/ugly without proper error handling; done
quickly late one evening to highly the behavior for a bug report.

For those who are curious, this code:
1. creates a partially sparse file
2. then 'maps' the file using SEEK_DATA/SEEK_HOLE

Showing how the map isn't stable even when the file contents (from a
VFS PoV) do not change (if tested on ZFS).

There is an argument for recently modified (or newly created) files
that a map consisting of one large data segment is a safe default.

By means of comparison if tested on XFS it will work as expected.
