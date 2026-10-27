#ifndef C2ID_SYS_STAT_H
#define C2ID_SYS_STAT_H
#include <sys/types.h>
struct stat { unsigned long st_dev; unsigned long st_ino; unsigned long st_nlink; unsigned int st_mode; unsigned int st_uid; unsigned int st_gid; int st_pad0; unsigned long st_rdev; long st_size; long st_blksize; long st_blocks; long st_atime; long st_atime_nsec; long st_mtime; long st_mtime_nsec; long st_ctime; long st_ctime_nsec; long st_unused[3]; };
#define S_IFMT 61440
#define S_IFREG 32768
#define S_IFDIR 16384
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_IRWXU 448
#define S_IRUSR 256
#define S_IWUSR 128
#define S_IXUSR 64
int stat(const char *path, struct stat *st);
int fstat(int fd, struct stat *st);
int mkdir(const char *path, unsigned int mode);
int chmod(const char *path, unsigned int mode);
#endif
