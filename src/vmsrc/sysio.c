#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include "plvm.h"

void syscall6502(M6502 *mpu)
{
    uword params;
    byte  cmd, status;

    PULL_ESTK(params);
    PULL_ESTK(cmd);
    status = 0;
    switch (cmd)
    {
    }
    PUSH_ESTK(status);
}
void sysopen(M6502 *mpu)
{
}
void sysclose(M6502 *mpu)
{
}
void sysread(M6502 *mpu)
{
}
void syswrite(M6502 *mpu)
{
}
void sysio_init(void)
{
}
