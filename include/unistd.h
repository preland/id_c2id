#ifndef C2ID_UNISTD_H
#define C2ID_UNISTD_H
#include <stddef.h>
#include <sys/types.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);
int close(int fd);
static ssize_t __c2id_w_read(int fd, void *buf, size_t n) { return read(fd, buf, n); }
static ssize_t __c2id_w_write(int fd, const void *buf, size_t n) { return write(fd, buf, n); }
static int __c2id_w_close(int fd) { return close(fd); }
#endif
