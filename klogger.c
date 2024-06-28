/****************************************************************************
**
** klogger.c
**
** Copyright (C) 2020 by KO Myung-Hun <komh@chollian.net>
**
** This file is part of K Soft Sequencer.
**
** $BEGIN_LICENSE$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**
** $END_LICENSE$
**
****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/fmutex.h>

#include "klogger.h"

void kloggerFdV(int depth, int fd, const char *format, va_list args)
{
    static _fmutex lock = _FMUTEX_INITIALIZER;

    char    msg[256];
    time_t  t;
    struct  tm tm;
    int     len;

    _fmutex_request(&lock, 0);

    time(&t);

    tm = *localtime( &t );

    len = snprintf(msg, sizeof(msg), "%04d-%02d-%02d %02d:%02d:%02d%*c",
                   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                   tm.tm_hour, tm.tm_min, tm.tm_sec,
                   depth * 2 + 1, ' ');

    len += vsnprintf(msg + len, sizeof(msg) - len, format, args);

    write(fd, msg, len);
    write(fd, "\n", 1);

    _fmutex_release(&lock);
}

void kloggerFd(int depth, int fd, const char *format, ...)
{
    va_list args;

    va_start(args, format);

    kloggerFdV(depth, fd, format, args);

    va_end(args);
}

void kloggerFileV(int depth, const char *file,
                  const char *format, va_list args)
{
    int     fd;

    fd = open(file, O_CREAT | O_WRONLY | O_APPEND | O_BINARY,
              S_IREAD | S_IWRITE);
    if (fd != -1)
    {
        kloggerFdV(depth, fd, format, args);

        close(fd);
    }
}

void kloggerFile(int depth, const char *file, const char *format, ...)
{
    va_list args;

    va_start(args, format);

    kloggerFileV(depth, file, format, args);

    va_end(args);
}

