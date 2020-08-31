#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/fmutex.h>

#include "klogger.h"

void kloggerFdV(int fd, const char *format, va_list args)
{
    static _fmutex lock = _FMUTEX_INITIALIZER;

    char    msg[256];
    time_t  t;
    struct  tm tm;
    int     len;

    _fmutex_request(&lock, 0);

    time(&t);

    tm = *localtime( &t );

    len = snprintf(msg, sizeof(msg), "%04d-%02d-%02d %02d:%02d:%02d ",
                   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                   tm.tm_hour, tm.tm_min, tm.tm_sec);

    len += vsnprintf(msg + len, sizeof(msg) - len, format, args);

    write(fd, msg, len);
    write(fd, "\n", 1);

    _fmutex_release(&lock);
}

void kloggerFd(int fd, const char *format, ...)
{
    va_list args;

    va_start(args, format);

    kloggerFdV(fd, format, args);

    va_end(args);
}

void kloggerFileV(const char *file, const char *format, va_list args)
{
    int     fd;

    fd = open(file, O_CREAT | O_WRONLY | O_APPEND | O_BINARY,
              S_IREAD | S_IWRITE);
    if (fd != -1)
    {
        kloggerFdV(fd, format, args);

        close(fd);
    }
}

void kloggerFile(const char *file, const char *format, ...)
{
    va_list args;

    va_start(args, format);

    kloggerFileV(file, format, args);

    va_end(args);
}

