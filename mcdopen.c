/****************************************************************************/
/*                                                                          */
/* SOURCE FILE NAME:  MCDOPEN.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_OPEN MESSAGE HANDLER                              */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCI_OPEN message.   */
/*            The processing of this message show allocate the instance     */
/*            structure and initialize it but NOT make it active.  The      */
/*            MCIDRV_RESTORE message will make it active.                   */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIOpen() - MCI_OPEN message handler                               */
/*       MCIOpenErr() - Error handler for MCI_OPEN message                  */
/****************************************************************************/
#define INCL_BASE                    // Base OS2 functions
#define INCL_DOSSEMAPHORES           // OS2 Semaphore function
#define INCL_MCIOS2                  // use the OS/2 like MMPM/2 headers

#include <os2.h>                     // OS2 defines.
#include <string.h>                  // C string functions
#include <os2me.h>                   // MME includes files.
#include <stdlib.h>                  // Math functions
#include "mcdtemp.h"                 // Function Prototypes.

/* callback for KAI */
static ULONG APIENTRY kaiCallback(PVOID pCBData,
                                  PVOID pBuffer, ULONG ulBufferSize)
{
    PINSTANCE pInst = pCBData;

    int written = kmdecDecode(pInst->dec, pBuffer, ulBufferSize);

    if (written < ulBufferSize && pInst->playNotify.hwndCallback)
    {
        mdmDriverNotify(pInst->usDeviceID,
                        pInst->playNotify.hwndCallback,
                        MM_MCINOTIFY, pInst->playNotify.usUserParm,
                        MAKEULONG(MCI_PLAY, MCI_NOTIFY_SUCCESSFUL));
    }

    int pos = kmdecGetPosition(pInst->dec);

    for (int i = 0; i < MAX_CUE_POINTS; i++)
    {
        CUENOTIFY *notify = &pInst->cueNotify[i];

        if (notify->On && !notify->Notified && notify->ulCuepoint <= pos)
        {
            notify->Notified = TRUE;

            mdmDriverNotify(pInst->usDeviceID,
                            notify->hwndCallback,
                            MM_MCICUEPOINT, notify->usUserParm,
                            MSECTOMM(notify->ulCuepoint));

        }
    }

    ULONG ulNext = pInst->adviseNotify.ulNext;

    if (ulNext > 0)
    {
        while (ulNext < pos)
        {
            mdmDriverNotify(pInst->usDeviceID,
                            pInst->adviseNotify.hwndCallback,
                            MM_MCIPOSITIONCHANGE,
                            pInst->adviseNotify.usUserParm,
                            MSECTOMM(ulNext));

            ulNext += pInst->adviseNotify.ulUnits;
        }

        pInst->adviseNotify.ulNext = ulNext;
    }

    return written;
}


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIOpen                                                */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_OPEN message processor                            */
/*                                                                          */
/* FUNCTION:  Process the MCI_OPEN message.                                 */
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
RC MCIOpen (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_INFO_PARMS      pInfoParms;               // Pointer to info structure
  PMMDRV_OPEN_PARMS    pDrvOpenParms;            // Pointer to MMDRV_OPEN structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pDrvOpenParms  = (PMMDRV_OPEN_PARMS)pFuncBlock->pParam2;

  /*******************************************************************/
  /* Allocate and initialize the instance structure                  */
  /*******************************************************************/
  if (!(pInstance = malloc(sizeof(INSTANCE))))
     ulrc = MCIERR_OUT_OF_MEMORY;
  else
     {
     if (DosCreateMutexSem(NULL,&(pInstance->hmtxAccessSem),DC_SEM_SHARED,FALSE))
        ulrc = MCIERR_DRIVER_INTERNAL;

     if (!ulrc)
        {
        /*******************************************************************/
        /* Set up default values in instance structure.                    */
        /*  NOTE this is a minimal instance structure                      */
        /*******************************************************************/
        memset(pInstance, 0, sizeof(INSTANCE));
        pInstance->usDeviceID = pDrvOpenParms->usDeviceID;
        pInstance->ulTimeFormat = MCI_FORMAT_MMTIME;
        pInstance->ulSpeedFormat = MCI_FORMAT_PERCENTAGE;
        pInstance->ulState = MCD_MODE_OPENING;
        pInstance->ulMode = MCI_MODE_NOT_READY;
        pInstance->Active = FALSE;
        pInstance->ulVolume = 75L;
        pInstance->ulMasterVolume = -1L;
        pInstance->Speaker = TRUE;
        pInstance->Headphone = TRUE;
        pInstance->usDeviceType = pDrvOpenParms->usDeviceType;
        pInstance->usDeviceOrd = pDrvOpenParms->usDeviceOrd;
        strcpy(pInstance->szDevParams, pDrvOpenParms->pDevParm);
        pDrvOpenParms->pInstance = pInstance;
        pDrvOpenParms->usResourceUnitsRequired = 1;
        pDrvOpenParms->usResourceClass = 1;
        GetINIInstallName(pInstance);
        GetDeviceInfo(pInstance);

        if (ulParam1 & MCI_OPEN_ELEMENT)
            strcpy(pInstance->szFileName, pDrvOpenParms->pszElementName);

        KMDECAUDIOINFO ai;

        ai.bps = KMDEC_BPS_S16;
        ai.channels = 2;
        ai.sampleRate = 44100;

        pInstance->dec = kmdecOpen(pInstance->szFileName, "e:/2gmgsmt.sf2",
                                   &ai);
        if (!pInstance->dec)
        {
            DosCloseMutexSem(pInstance->hmtxAccessSem);

            free(pInstance);

            return MCIERR_DRIVER_INTERNAL;
        }

        KAISPEC ksWanted, ksObtained;

        ksWanted.usDeviceIndex = 0;
        ksWanted.ulType = KAIT_PLAY;
        ksWanted.ulBitsPerSample = BPS_16;
        ksWanted.ulSamplingRate = ai.sampleRate;
        ksWanted.ulDataFormat = 0;
        ksWanted.ulChannels = ai.channels;
        ksWanted.ulNumBuffers = 2;
        ksWanted.ulBufferSize = 4096 * 2 * 2; /* samples * 16bits * 2 ch */
        ksWanted.fShareable = ulParam1 & MCI_OPEN_SHAREABLE;
        ksWanted.pfnCallBack = kaiCallback;
        ksWanted.pCallBackData = pInstance;

        if (kaiOpen(&ksWanted, &ksObtained, &pInstance->hkai))
        {
            kmdecClose(pInstance->dec);

            DosCloseMutexSem(pInstance->hmtxAccessSem);

            free(pInstance);

            return MCIERR_DRIVER_INTERNAL;
        }

        kaiEnableSoftVolume(pInstance->hkai, TRUE);
        }
     }


  return (ulrc);

}      /* end of MCIOpen */


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIOpenErr                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_OPEN message processor for errors                 */
/*                                                                          */
/* FUNCTION:  Process the MCI_OPEN message for errors                       */
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
RC MCIOpenErr (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_INFO_PARMS      pInfoParms;               // Pointer to info structure
  PMMDRV_OPEN_PARMS    pDrvOpenParms;            // Pointer to MMDRV_OPEN structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pDrvOpenParms  = (PMMDRV_OPEN_PARMS)pFuncBlock->pParam2;

  /*******************************************************/
  /* Validate that we have only valid flags              */
  /*******************************************************/
  if (ulParam1 & ~(MCIOPENVALIDFLAGS))
     return(MCIERR_INVALID_FLAG);

  /* support MCI_OPEN_ELEMENT only */
  if (!(ulParam1 & MCI_OPEN_ELEMENT))
     return(MCIERR_UNSUPPORTED_FLAG);


  return (ulrc);

}      /* end of MCIOpenErr */

