/****************************************************************************
**
** mcdresume.c
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

/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDRESUME.C                                           */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_RESUME MESSAGE HANDLER                            */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_RESUME message. */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIResume() - MCI_RESUME message handler                           */
/****************************************************************************/
#define INCL_BASE                    // Base OS2 functions
#define INCL_DOSSEMAPHORES           // OS2 Semaphore function
#define INCL_MCIOS2                  // use the OS/2 like MMPM/2 headers

#include <os2.h>                     // OS2 defines.
#include <string.h>                  // C string functions
#include <os2me.h>                   // MME includes files.
#include <stdlib.h>                  // Math functions
#include "mcdtemp.h"                 // Function Prototypes.

/***********************************************/
/* MCI_RESUME valid flags                  */
/***********************************************/
#define MCIRESUMEVALIDFLAGS     (MCI_WAIT | MCI_NOTIFY)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIResume                                              */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_RESUME message processor                          */
/*                                                                          */
/* FUNCTION:  Process the MCI_RESUME message.                               */
/*                                                                          */
/* PARAMETERS:                                                              */
/*      FUNCTION_PARM_BLOCK  *pFuncBlock -- Pointer to function parameter   */
/*                                          block.                          */
/* EXIT CODES:                                                              */
/*      MCIERR_SUCCESS    -- Action completed without error.                */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*                                                                          */
/****************************************************************************/
RC MCIResume(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_GENERIC_PARMS  pParam2;                // Pointer to GENERIC structure
    PINSTANCE           pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    LOG_ENTER(++pInst->ulDepth, "ulParam1 = 0x%lx", ulParam1);

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCIRESUMEVALIDFLAGS))
        LOG_RETURN(pInst->ulDepth--, MCIERR_INVALID_FLAG);

    kaiResume(pInst->hkai);

    /***************************************************************/
    /* Send back a notification if the notify flag was on          */
    /***************************************************************/
    if (ulParam1 & MCI_NOTIFY)
        rc = mdmDriverNotify(pInst->usDeviceID,
                             pParam2->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG(MCI_RESUME, MCI_NOTIFY_SUCCESSFUL));

    LOG_RETURN(pInst->ulDepth--, rc);
}
