/****************************************************************************
**
** mcdpos.c
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
/* SOURCE FILE NAME:  MCDPOS.C                                              */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SET_POSITION_ADVISE MESSAGE HANDLER               */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the                     */
/*            MCI_SET_POSITION_ADVISE message.                              */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCISetPositionAdvise() - MCI_SET_POSITION_ADVISE message handler   */
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
/* MCI_SET_POSITION_ADVISE valid flags                  */
/***********************************************/
#define MCISETPOSITIONADVISEVALIDFLAGS  (MCI_WAIT | MCI_NOTIFY | \
                                         MCI_SET_POSITION_ADVISE_ON | \
                                         MCI_SET_POSITION_ADVISE_OFF)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCISetPositionAdvise                                   */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SET_POSITION_ADVISE message processor             */
/*                                                                          */
/* FUNCTION:  Process the MCI_SET_POSITION_ADVISE message.                  */
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
RC MCISetPositionAdvise(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_POSITION_PARMS pParam2;                // Pointer to POSITION structure
    PINSTANCE           pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    LOG_ENTER(++pInst->ulDepth,
              "ulParam1 = 0x%lx, ulUnits = %ld, usUserParm = %d",
              ulParam1, pParam2->ulUnits, pParam2->usUserParm);

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCISETPOSITIONADVISEVALIDFLAGS))
        LOG_RETURN(pInst->ulDepth--, MCIERR_INVALID_FLAG);

    switch (ulParam1 & ~(MCI_WAIT | MCI_NOTIFY))
    {
        case MCI_SET_POSITION_ADVISE_ON:
        {
            ULONG ulUnits = ConvertTime(pParam2->ulUnits, pInst->ulTimeFormat,
                                        MCI_FORMAT_MILLISECONDS);

            if (ulUnits > 0)
            {
                int pos = kmdecGetPosition(pInst->dec);

                pInst->adviseNotify.hwndCallback = pParam2->hwndCallback;
                pInst->adviseNotify.ulUnits = ulUnits;
                pInst->adviseNotify.usUserParm = pParam2->usUserParm;
                pInst->adviseNotify.ulNext = ((pos + ulUnits - 1) / ulUnits) *
                                             ulUnits;
            }
            else
                rc = MCIERR_OUTOFRANGE;

            break;
        }

        case MCI_SET_POSITION_ADVISE_OFF:
            pInst->adviseNotify.ulUnits = 0;
            pInst->adviseNotify.ulNext = 0;
            break;
    }

    /***************************************************************/
    /* Send back a notification if the notify flag was on          */
    /***************************************************************/
    if ((ulParam1 & MCI_NOTIFY) && !rc)
        rc = mdmDriverNotify(pInst->usDeviceID,
                             pParam2->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG(MCI_SET_POSITION_ADVISE,
                                       MCI_NOTIFY_SUCCESSFUL));

    LOG_RETURN(pInst->ulDepth--, rc);
}
