#ifndef C2ID_SYS_WAIT_H
#define C2ID_SYS_WAIT_H
#include <sys/types.h>
#define WNOHANG 1
#define WIFEXITED(s) (((s) & 127) == 0)
#define WEXITSTATUS(s) (((s) >> 8) & 255)
#define WIFSIGNALED(s) (((s) & 127) != 0 && ((s) & 127) != 127)
#define WTERMSIG(s) ((s) & 127)
int waitpid(int pid, int *status, int options);
int wait(int *status);
#endif
