/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDPLAY.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_PLAY MESSAGE HANDLER                              */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_PLAY message.   */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIPlay() - MCI_PLAY message handler                               */
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
/* MCI_PLAY valid flags                  */
/***********************************************/
#define MCIPLAYVALIDFLAGS   (MCI_WAIT | MCI_NOTIFY | MCI_FROM | MCI_TO)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIPlay                                                */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_PLAY message processor                            */
/*                                                                          */
/* FUNCTION:  Process the MCI_PLAY message.                                 */
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
RC MCIPlay(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_PLAY_PARMS     pParam2;                // Pointer to PLAY structure
    PINSTANCE           pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    LOG_ENTER("ulParam1 = 0x%lx", ulParam1);

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCIPLAYVALIDFLAGS))
        LOG_RETURN(MCIERR_INVALID_FLAG);

    /* MCI_FROM and MCI_TO are not supported yet */
    if (ulParam1 & (MCI_FROM | MCI_TO))
        LOG_RETURN(MCIERR_UNSUPPORTED_FLAG);

    DosRequestMutexSem(pInst->hmtxAccessSem, -2);

    pInst->playNotify.hwndCallback = (ulParam1 & MCI_NOTIFY) ?
                                     pParam2->hwndCallback : NULLHANDLE;
    pInst->playNotify.usUserParm = pFuncBlock->usUserParm;

    kaiPlay(pInst->hkai);

    if (ulParam1 & MCI_WAIT)
    {
        while (kaiStatus(pInst->hkai) & KAIS_PLAYING)
            DosSleep(1);
    }

    DosReleaseMutexSem(pInst->hmtxAccessSem);

    /* notifying is done in kaiCallback() in mcdopen.c */

    LOG_RETURN(rc);
}
