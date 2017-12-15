/* unholey
 *
 * show SEEK_DATA / SEEK_HOLE not working even for reads; debug using
 * strace
 *
 * https://github.com/zfsonlinux/zfs/issues/6958
 *
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
// #include <sys/types.h>

void fail(const char *c)
{
    perror(c);
    exit(3);
}

void dump_segments(const char *f, const char *c)
{
    struct stat st;

    printf("\n%s\n", c);
    for (int l = strlen(c); l>0; l--)
	printf("-");

    int fd = open(f, O_RDONLY);
    if (fd < 0)
	fail("can't open file for read (seeking)");

    if (fstat(fd, &st) < 0)
	fail("can't stat");
    off_t flen = st.st_size;

    off_t prevOfs = 0, newOfs;
    int   prevOp = SEEK_HOLE, curOp;
    int   segments = 0;
    for (;;) {
	lseek(fd, 0, SEEK_SET);
	curOp = (prevOp == SEEK_DATA)?SEEK_HOLE:SEEK_DATA;

	newOfs = lseek(fd, prevOfs, curOp);

	if (newOfs == -1) {
	    if (errno==ENXIO)
		break;
	    fail("lseek");
	}

	if (prevOfs != newOfs) {
	    segments++;
	    off_t w = newOfs - prevOfs;
	    const char *t = "HOLE";
	    if (curOp != SEEK_DATA)
		t = "DATA";

	    printf("\n%s %12ld %12ld (%12ld)", t, prevOfs, newOfs, w);
	}

	if (newOfs >= flen)
	    break;

	prevOfs = newOfs;
	prevOp = curOp;
    }
    printf("\t\t%d segments\n", segments);
}

int main(int ac, char *av[])
{
    int fd;
    const char *filename = "/tank0/cache/cw/a-bit-sparse";

    if (ac == 2) {
	filename = av[1];
    }

    fd = open(filename, O_RDWR|O_CREAT|O_EXCL|O_TRUNC, 0755);
    if (fd < 0)
	fail("unable to create new file");

    // what errors?
    lseek(fd, 1*1024*1024, SEEK_SET);
    write(fd, "x", 1);

    lseek(fd, 2*1024*1024, SEEK_SET);
    write(fd, "y", 1);

    ftruncate(fd, 3*1024*1024);
    close(fd);

    // this will show 1 segment (expected)
    dump_segments(filename, "newly created");

    // wait a bit ... this should show 4 segments (expected)
    sleep(6);
    dump_segments(filename, "after a delay");

    // test read using original fd
    char buf[1];
    fd = open(filename, O_RDWR);
    if (fd < 0)
	fail("unable to open");
    pread(fd, &buf, 1, 0);
    dump_segments(filename, "immediately after a read");

    // wait some more, verify it's normal again
    sleep(6);
    dump_segments(filename, "delay...");

    // not read without atime
    int f2 = open(filename, O_RDONLY|O_NOATIME);
    pread(f2, &buf, 1, 0);
    dump_segments(filename, "immediately after a read using NOATIME");

    return 0;
}
