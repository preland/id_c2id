#ifndef C2ID_TERMIOS_H
#define C2ID_TERMIOS_H
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
struct termios { tcflag_t c_iflag, c_oflag, c_cflag, c_lflag; cc_t c_cc[32]; };
#define TCSANOW 0
#define ICANON 2
#define ECHO 8
int tcgetattr(int fd, struct termios *t);
int tcsetattr(int fd, int act, const struct termios *t);
#endif
