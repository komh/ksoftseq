/****************************************************************************
**
** malloc.c
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

#define INCL_DOS
#include <os2.h>

#include <string.h>

#include <emx/umalloc.h>

#include "mcdtemp.h"

#define MIN_OF_DOSALLOCMEM  ( 64 * 1024 )

struct PTRINFO
{
    void *magic;
    size_t size;
};

void *malloc(size_t size)
{
    struct PTRINFO *p;

    if (size < MIN_OF_DOSALLOCMEM)
        p = _hmalloc(size + sizeof(*p));
    else
    {
        if (DosAllocMem((PPVOID)&p, size + sizeof(*p),
                        fPERM | PAG_COMMIT | OBJ_ANY))
            p = NULL;

        LOG_MSG(2, "DosAllocMem(%d) = %p", size, p + 1);
    }

    if (p)
    {
        p->magic = size < MIN_OF_DOSALLOCMEM ?
                        (void *)_hmalloc : (void *)DosAllocMem;
        p->size = size;

        return p + 1;
    }

    return NULL;
}

void *calloc(size_t elements, size_t size)
{
    void *p = malloc(elements * size);

    if (p)
        memset(p, 0, elements * size);

    return p;
}

void *_std_realloc(void *, size_t);

void *realloc(void *mem, size_t size)
{
    if (!mem)
        return malloc(size);

    if (mem && !size)
    {
        free(mem);

        return NULL;
    }

    struct PTRINFO *p = mem;
    p--;

    /*
     * If memory block was not allocated by _hmalloc() nor DosAllocMem(),
     * use _std_realloc() because it's not possible to know size of mem.
     */
    if (p->magic != _hmalloc && p->magic != DosAllocMem)
        return _std_realloc(mem, size);

    void *newMem = malloc(size);

    if (!newMem)
        return NULL;

    if (size > p->size)
        size = p->size;

    memcpy(newMem, mem, size);

    free(mem);

    return newMem;
}

void _std_free(void *);

void free(void *mem)
{
    if (!mem)
        return;

    struct PTRINFO *p = mem;
    p--;

    if (p->magic == _hmalloc)
        _std_free(p);
    else if (p->magic == DosAllocMem)
    {
        int size = p->size;

        LOG_MSG(2, "DosFreeMem(%p, %d) = %ld", mem, size, DosFreeMem(p));
    }
    else
        _std_free(mem);
}
