#ifndef KLOGGER_H
#define KLOGGER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void kloggerFdV(int fd, const char *format, va_list args);
void kloggerFd(int fd, const char *format, ...);
void kloggerFileV(const char *file, const char *format, va_list args);
void kloggerFile(const char *file, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
