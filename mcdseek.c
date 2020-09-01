/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDSEEK.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SEEK MESSAGE HANDLER                              */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_SEEK message.   */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCISeek() - MCI_SEEK message handler                               */
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
/* MCI_SEEK valid flags                  */
/***********************************************/
#define MCISEEKVALIDFLAGS   (MCI_WAIT | MCI_NOTIFY | \
                             MCI_TO | MCI_TO_START | MCI_TO_END)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCISeek                                                */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SEEK message processor                            */
/*                                                                          */
/* FUNCTION:  Process the MCI_SEEK message.                                 */
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
RC MCISeek(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_SEEK_PARMS     pParam2;                // Pointer to SEEK structure
    PINSTANCE           pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    LOG_ENTER("ulParam1 = 0x%lx, ulTo = %ld",
              ulParam1, pParam2->ulTo);

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCISEEKVALIDFLAGS))
        LOG_RETURN(MCIERR_INVALID_FLAG);

    int duration = kmdecGetDuration(pInst->dec);
    int to;

    if (ulParam1 & MCI_TO)
        to = ConvertTime(pParam2->ulTo, pInst->ulTimeFormat,
                         MCI_FORMAT_MILLISECONDS);
    else if (ulParam1 & MCI_TO_START)
        to = 0;
    else /* if (ulParam1 & MCI_TO_END) */
        to = duration;

    if (to > duration)
        rc = MCIERR_OUTOFRANGE;
    else
    {
        kmdecSeek(pInst->dec, to, KMDEC_SEEK_SET);

        /* reset cutepoint notified flag */
        for (int i = 0; i < MAX_CUE_POINTS; i++)
            pInst->cueNotify[i].Notified = pInst->cueNotify[i].ulCuepoint < to;

        ULONG ulUnits = pInst->adviseNotify.ulUnits;

        if (ulUnits > 0)
        {
            pInst->adviseNotify.ulNext = ((to + ulUnits - 1) / ulUnits) *
                                         ulUnits;
        }
    }

    /***************************************************************/
    /* Send back a notification if the notify flag was on          */
    /***************************************************************/
    if ((ulParam1 & MCI_NOTIFY) && !rc)
        rc = mdmDriverNotify(pInst->usDeviceID,
                             pParam2->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG(MCI_SEEK, MCI_NOTIFY_SUCCESSFUL));

    LOG_RETURN(rc);
}
