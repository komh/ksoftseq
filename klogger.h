/****************************************************************************
**
** klogger.h
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

#ifndef KLOGGER_H
#define KLOGGER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void kloggerFdV(int depth, int fd, const char *format, va_list args);
void kloggerFd(int depth, int fd, const char *format, ...);
void kloggerFileV(int depth, const char *file,
                  const char *format, va_list args);
void kloggerFile(int depth, const char *file, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
