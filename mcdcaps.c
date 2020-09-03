/****************************************************************************
**
** mcdcaps.c
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
/* SOURCE FILE NAME:  MCDCAPS.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_GETDEVCAPS MESSAGE HANDLER                        */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_GETDEVCAPS      */
/*            message.                                                      */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCICaps() - MCI_GETDEVCAPS message handler                         */
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
/* MCI_GETDEVCAPS valid flags                  */
/***********************************************/
#define MCIGETDEVCAPSVALIDFLAGS     (MCI_WAIT | MCI_NOTIFY | \
                                     MCI_GETDEVCAPS_MESSAGE | \
                                     MCI_GETDEVCAPS_ITEM)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCICaps                                                */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_GETDEVCAPS message processor                      */
/*                                                                          */
/* FUNCTION:  Process the MCI_GETDEVCAPS message.                           */
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
RC MCICaps(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG                   rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG                   ulParam1;               // Message flags
    PMCI_GETDEVCAPS_PARMS   pParam2;                // Pointer to GETDEVCAPS structure
    PINSTANCE               pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    LOG_ENTER("ulParam1 = 0x%lx, usMessage = %d, ulItem = %d",
              ulParam1, pParam2->usMessage, pParam2->ulItem);

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCIGETDEVCAPSVALIDFLAGS))
        LOG_RETURN(MCIERR_INVALID_FLAG);

    switch (ulParam1 & ~(MCI_WAIT | MCI_NOTIFY))
    {
        case MCI_GETDEVCAPS_MESSAGE:
            switch (pParam2->usMessage)
            {
                /* supported messages */
                case MCI_CLOSE:
                case MCI_GETDEVCAPS:
                case MCI_INFO:
                case MCI_LOAD:
                case MCI_OPEN:
                case MCI_PAUSE:
                case MCI_PLAY:
                case MCI_RESUME:
                case MCI_SEEK:
                case MCI_SET:
                case MCI_SET_CUEPOINT:
                case MCI_SET_POSITION_ADVISE:
                case MCI_STATUS:
                case MCI_STOP:
                    pParam2->ulReturn = MCI_TRUE;
                    rc = MAKEULONG(rc, MCI_TRUE_FALSE_RETURN);
                    break;

                /* unsupported messages */
                case MCI_ACQUIREDEVICE:
                case MCI_CONNECTOR:
                case MCI_CUE:
                case MCI_DEVICESETTINGS:
                case MCI_ESCAPE:
                case MCI_GETTOC:
                case MCI_MASTERAUDIO:
                case MCI_RECORD:
                case MCI_RELEASEDEVICE :
                case MCI_SAVE:
                case MCI_SET_SYNC_OFFSET:
                case MCI_SPIN:
                case MCI_STEP:
                case MCI_SYSINFO:
                case MCI_UPDATE:
                    pParam2->ulReturn = MCI_FALSE;
                    rc = MAKEULONG(rc, MCI_TRUE_FALSE_RETURN);
                    break;

                /* empty message */
                case 0:
                default:
                    pParam2->ulReturn = MCI_FALSE;
                    rc = MCIERR_INVALID_FLAG;
                    break;
            }

            break;

        case MCI_GETDEVCAPS_ITEM:
            switch (pParam2->ulItem)
            {
                /* supported items */
                case MCI_GETDEVCAPS_CAN_PLAY:
                case MCI_GETDEVCAPS_CAN_SETVOLUME:
                case MCI_GETDEVCAPS_HAS_AUDIO:
                case MCI_GETDEVCAPS_USES_FILES:
                    pParam2->ulReturn = MCI_TRUE;
                    rc = MAKEULONG(rc, MCI_TRUE_FALSE_RETURN);
                    break;

                case MCI_GETDEVCAPS_DEVICE_TYPE:
                    pParam2->ulReturn = MCI_DEVTYPE_SEQUENCER;
                    rc = MAKEULONG(rc, MCI_DEVICENAME_RETURN);
                    break;

                case MCI_GETDEVCAPS_PREROLL_TIME:
                    pParam2->ulReturn = 0;
                    rc = MAKEULONG(rc, MCI_INTEGER_RETURNED);
                    break;

                case MCI_GETDEVCAPS_PREROLL_TYPE:
                    pParam2->ulReturn = MCI_PREROLL_NONE;
                    rc = MAKEULONG(rc, MCI_PREROLL_TYPE_RETURN);
                    break;

                /* unsupported items */
                case MCI_GETDEVCAPS_CAN_EJECT:
                case MCI_GETDEVCAPS_CAN_LOCKEJECT:
                case MCI_GETDEVCAPS_CAN_PROCESS_INTERNAL:
                case MCI_GETDEVCAPS_CAN_RECORD:
                case MCI_GETDEVCAPS_CAN_RECORD_INSERT:
                case MCI_GETDEVCAPS_CAN_SAVE:
                case MCI_GETDEVCAPS_CAN_STREAM:
                case MCI_GETDEVCAPS_HAS_IMAGE:
                case MCI_GETDEVCAPS_HAS_VIDEO:
                    pParam2->ulReturn = MCI_FALSE;
                    rc = MAKEULONG(rc, MCI_TRUE_FALSE_RETURN);
                    break;

                case 0:
                default:
                    pParam2->ulReturn = MCI_FALSE;
                    rc = MCIERR_INVALID_ITEM_FLAG;
                    break;
            }

            break;

        case 0:
            rc = MCIERR_MISSING_FLAG;
            break;

        case MCI_GETDEVCAPS_MESSAGE | MCI_GETDEVCAPS_ITEM:
            rc = MCIERR_FLAGS_NOT_COMPATIBLE;
            break;

        default:
            rc = MCIERR_UNSUPPORTED_FLAG;
            break;
    }

    /***************************************************************/
    /* Send back a notification if the notify flag was on          */
    /***************************************************************/
    if ((ulParam1 & MCI_NOTIFY) && !ULONG_LOWD(rc))
        rc = mdmDriverNotify(pInst->usDeviceID,
                             pParam2->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG(MCI_GETDEVCAPS, MCI_NOTIFY_SUCCESSFUL));

    LOG_RETURN(rc);
}
