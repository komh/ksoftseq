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

#include <emx/umalloc.h>

void *malloc(size_t size)
{
    void *ptr = _hmalloc(size);

    return ptr ? ptr : _lmalloc(size);
}

void *calloc(size_t elements, size_t size)
{
    void *ptr = _hcalloc(elements, size);

    return ptr ? ptr : _lcalloc(elements, size);
}

void *realloc(void *mem, size_t size)
{
    void *ptr = _hrealloc(mem, size);

    return ptr ? ptr : _lrealloc(mem, size);
}
