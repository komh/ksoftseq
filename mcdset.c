/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDSET.C                                              */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SET MESSAGE HANDLER                               */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_SET message.    */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCISet() - MCI_SET message handler                                 */
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
/* MCI_SET valid flags                  */
/***********************************************/
#define MCISETVALIDFLAGS   (MCI_WAIT | MCI_NOTIFY | \
                            MCI_SET_AUDIO | MCI_SET_VOLUME | \
                            MCI_SET_ON | MCI_SET_OFF | \
                            MCI_SET_TIME_FORMAT)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCISet                                                 */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_SET message processor                             */
/*                                                                          */
/* FUNCTION:  Process the MCI_SET message.                                  */
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
RC MCISet(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_SET_PARMS      pParam2;                // Pointer to SET structure
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
    if (ulParam1 & ~(MCISETVALIDFLAGS))
        return MCIERR_INVALID_FLAG;

    DosRequestMutexSem(pInst->hmtxAccessSem, -2);

    switch (ulParam1 & ~(MCI_WAIT | MCI_NOTIFY))
    {
        case MCI_SET_AUDIO | MCI_SET_ON:
        case MCI_SET_AUDIO | MCI_SET_OFF:
            kaiSetSoundState(pInst->hkai, pParam2->ulAudio,
                             ulParam1 & MCI_SET_ON);
            break;

        case MCI_SET_AUDIO | MCI_SET_VOLUME:
            kaiSetVolume(pInst->hkai, pParam2->ulAudio, pParam2->ulLevel);
            break;

        case MCI_SET_TIME_FORMAT:
            switch (pParam2->ulTimeFormat)
            {
                case MCI_FORMAT_MILLISECONDS:
                case MCI_FORMAT_MMTIME:
                    pInst->ulTimeFormat = pParam2->ulTimeFormat;
                    break;

                case MCI_SEQ_SET_SMPTE_24:
                case MCI_SEQ_SET_SMPTE_25:
                case MCI_SEQ_SET_SMPTE_30:
                case MCI_SEQ_SET_SMPTE_30DROP:
                case MCI_SEQ_SET_SONGPTR:
                default:
                    rc = MCIERR_INVALID_TIME_FORMAT_FLAG;
                    break;
            }
            break;

        case MCI_SEQ_SET_MASTER:
        case MCI_SEQ_SET_OFFSET:
        case MCI_SEQ_SET_PORT:
        case MCI_SEQ_SET_SLAVE:
        case MCI_SEQ_SET_TEMPO:
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
                             MAKEULONG(MCI_SET, MCI_NOTIFY_SUCCESSFUL));

    return rc;
}
