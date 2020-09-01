/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDLOAD.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_LOAD MESSAGE HANDLER                              */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_LOAD message.   */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCILoad() - MCI_LOAD message handler                               */
/****************************************************************************/
#define INCL_BASE                    // Base OS2 functions
#define INCL_DOSSEMAPHORES           // OS2 Semaphore function
#define INCL_MCIOS2                  // use the OS/2 like MMPM/2 headers

#include <os2.h>                     // OS2 defines.
#include <string.h>                  // C string functions
#include <os2me.h>                   // MME includes files.
#include <stdlib.h>                  // Math functions
#include "mcdtemp.h"                 // Function Prototypes.

#include <sys/stat.h>                // stat()

/***********************************************/
/* MCI_LOAD valid flags                  */
/***********************************************/
#define MCILOADVALIDFLAGS   (MCI_WAIT | MCI_NOTIFY | \
                             MCI_OPEN_ELEMENT | MCI_OPEN_MMIO)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCILoad                                                */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_LOAD message processor                            */
/*                                                                          */
/* FUNCTION:  Process the MCI_LOAD message.                                 */
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
RC MCILoad(FUNCTION_PARM_BLOCK *pFuncBlock)
{
    ULONG               rc = MCIERR_SUCCESS;    // Propogated Error Code
    ULONG               ulParam1;               // Message flags
    PMCI_LOAD_PARMS     pParam2;                // Pointer to LOAD structure
    PINSTANCE           pInst;                  // Pointer to instance

    /*****************************************************/
    /* dereference the values from pFuncBlock            */
    /*****************************************************/
    ulParam1    = pFuncBlock->ulParam1;
    pParam2     = pFuncBlock->pParam2;
    pInst       = pFuncBlock->pInstance;

    LOG_ENTER("ulParam1 = 0x%lx, pszElementName = %p, [%s]",
              ulParam1, pParam2->pszElementName,
              ulParam1 & MCI_OPEN_ELEMENT ? pParam2->pszElementName : "");

    /*******************************************************/
    /* Validate that we have only valid flags              */
    /*******************************************************/
    if (ulParam1 & ~(MCILOADVALIDFLAGS))
        LOG_RETURN(MCIERR_INVALID_FLAG);

    kaiStop(pInst->hkai);

    kmdecClose(pInst->dec);

    KMDECAUDIOINFO ai;

    ai.bps = KMDEC_BPS_S16;
    ai.channels = 2;
    ai.sampleRate = 44100;

    const char *sf2 = getenv("KSOFTSEQ_SF2");
    struct stat st;

    if (!sf2 || stat(sf2, &st) == -1)
        sf2 = szDefaultSf2;

    LOG_MSG("sf2 = [%s]", sf2);

    if (ulParam1 & MCI_OPEN_ELEMENT)
    {
        strcpy(pInst->szFileName, pParam2->pszElementName);

        pInst->dec = kmdecOpen(pInst->szFileName, sf2, &ai);
    }
    else /* if (ulParam1 & MCI_OPEN_MMIO) */
    {
        HMMIO fd = (HMMIO)pParam2->pszElementName;
        extern KMDECIOFUNCS io;

        pInst->dec = kmdecOpenFdEx(fd, sf2, &ai, &io);
    }

    if (!pInst->dec)
        rc = MCIERR_DRIVER_INTERNAL;

    memset(pInst->cueNotify, 0, sizeof(pInst->cueNotify));

    pInst->adviseNotify.ulUnits = 0;
    pInst->adviseNotify.ulNext = 0;

    /***************************************************************/
    /* Send back a notification if the notify flag was on          */
    /***************************************************************/
    if ((ulParam1 & MCI_NOTIFY) && !rc)
        rc = mdmDriverNotify(pInst->usDeviceID,
                             pParam2->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG(MCI_LOAD, MCI_NOTIFY_SUCCESSFUL));

    LOG_RETURN(rc);
}
