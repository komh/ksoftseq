/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDCUE.C                                              */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SETCUEPOINT MESSAGE HANDLER                       */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_SETCUEPOINT     */
/*            message.                                                      */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCISetCuePoint() - MCI_SETCUEPOINT message handler                 */
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
/* MCI_SETCUEPOINT valid flags                  */
/***********************************************/
#define MCISETCUEPOINTVALIDFLAGS    (MCI_WAIT | MCI_NOTIFY | \
                                     MCI_SET_CUEPOINT_ON | \
                                     MCI_SET_CUEPOINT_OFF)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCISetCuePoint                                         */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SETCUEPOINT message processor                     */
/*                                                                          */
/* FUNCTION:  Process the MCI_SETCUEPOINT message.                          */
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
RC MCISetCuePoint(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_CUEPOINT_PARMS pParam2;                // Pointer to CUEPOINT structure
    PINSTANCE           pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCISETCUEPOINTVALIDFLAGS))
        return MCIERR_INVALID_FLAG;

    DosRequestMutexSem(pInst->hmtxAccessSem, -2);

    ULONG ulCuepoint = ConvertTime(pParam2->ulCuepoint, pInst->ulTimeFormat,
                                   MCI_FORMAT_MILLISECONDS);

    switch (ulParam1 & ~(MCI_WAIT | MCI_NOTIFY))
    {
        case MCI_SET_CUEPOINT_ON:
        {
            int empty = -1;

            for (int i = 0; i < MAX_CUE_POINTS; i++)
            {
                if (pInst->cueNotify[i].On)
                {
                    if (pInst->cueNotify[i].ulCuepoint == ulCuepoint)
                    {
                        rc = MCIERR_DUPLICATE_CUEPOINT;
                        break;
                    }
                }
                else if (empty == -1)
                    empty = i;          /* first empty slot */
            }

            if (!rc)
            {
                if (empty == -1)
                    rc = MCIERR_CUEPOINT_LIMIT_REACHED;
                else
                {
                    pInst->cueNotify[empty].hwndCallback =
                        pParam2->hwndCallback;
                    pInst->cueNotify[empty].ulCuepoint = ulCuepoint;
                    pInst->cueNotify[empty].usUserParm = pParam2->usUserParm;
                    pInst->cueNotify[empty].On = TRUE;
                }
            }

            break;
        }

        case MCI_SET_CUEPOINT_OFF:
        {
            int i;

            for (i = 0; i < MAX_CUE_POINTS; i++)
            {
                if (pInst->cueNotify[i].On &&
                        pInst->cueNotify[i].ulCuepoint == ulCuepoint)
                {
                    pInst->cueNotify[i].On = FALSE;
                    break;
                }
            }

            if (i == MAX_CUE_POINTS)
                rc = MCIERR_INVALID_CUEPOINT;

            break;
        }

        default:
            rc = MCIERR_UNSUPPORTED_FLAG;
            break;
    }

    DosReleaseMutexSem(pInst->hmtxAccessSem);

    /***************************************************************/
    /* Send back a notification if the notify flag was on          */
    /***************************************************************/
    if ((ulParam1 & MCI_NOTIFY) && !rc)
        rc = mdmDriverNotify(pInst->usDeviceID,
                             pParam2->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG(MCI_SET_CUEPOINT,
                                       MCI_NOTIFY_SUCCESSFUL));

    return rc;
}
