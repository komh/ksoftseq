/****************************************************************************
**
** mcdopen.c
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
/****************************************************************************/
#define INCL_BASE                    // Base OS2 functions
#define INCL_DOSSEMAPHORES           // OS2 Semaphore function
#define INCL_MCIOS2                  // use the OS/2 like MMPM/2 headers

#include <os2.h>                     // OS2 defines.
#include <string.h>                  // C string functions
#include <os2me.h>                   // MME includes files.
#include <stdlib.h>                  // Math functions
#include "mcdtemp.h"                 // Function Prototypes.

#include <errno.h>                   // errno, EINVAL
#include <sys/stat.h>                // stat()

/* callback for KAI */
static ULONG APIENTRY kaiCallback(PVOID pCBData,
                                  PVOID pBuffer, ULONG ulBufferSize)
{
    PINSTANCE pInst = pCBData;
    ULONG rc = ERROR_TIMEOUT;

    /* prevent dead-lock in MCIPlay(), MCIStop() and MCIClose() */
    while (!pInst->AvoidDeadLock && rc == ERROR_TIMEOUT)
        rc = DosRequestMutexSem(pInst->hmtxAccessSem, 500);

    int written = kmdecDecode(pInst->dec, pBuffer, ulBufferSize);

    int pos = kmdecGetPosition(pInst->dec);

    if (pInst->ulEndPosition && pos > pInst->ulEndPosition)
        written = 0;

    if (written < ulBufferSize && pInst->playNotify.hwndCallback)
    {
        mdmDriverNotify(pInst->usDeviceID,
                        pInst->playNotify.hwndCallback,
                        MM_MCINOTIFY, pInst->playNotify.usUserParm,
                        MAKEULONG(MCI_PLAY, MCI_NOTIFY_SUCCESSFUL));
    }

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

    if (!rc)
        DosReleaseMutexSem(pInst->hmtxAccessSem);

    return written;
}

static int ioRead(int fd, void *buf, size_t n)
{
    ULONG rc = mmioRead(fd, buf, n);

    return rc == MMIO_ERROR ? -1 : rc;
}

static int ioSeek(int fd, long offset, int origin)
{
    static LONG lOrigins[] = { SEEK_SET, SEEK_CUR, SEEK_END };

    if( origin >= sizeof( lOrigins ) / sizeof( lOrigins[ 0 ]))
    {
        errno = EINVAL;
        return -1;
    }

    ULONG rc = mmioSeek(fd, offset, lOrigins[origin]);

    return rc == MMIO_ERROR ? -1 : rc;
}

static int ioTell(int fd)
{
    ULONG rc = mmioSeek(fd, 0, SEEK_CUR);

    return rc == MMIO_ERROR ? -1 : rc;
}

KMDECIOFUNCS io = {
    .open = NULL,
    .read = ioRead,
    .seek = ioSeek,
    .tell = ioTell,
    .close = NULL
};

/***********************************************/
/* MCI_OPEN valid flags                        */
/*  NOTE --> MCI_NOTIFY will never be sent     */
/*           open notify is handled by MDM     */
/***********************************************/
#define MCIOPENVALIDFLAGS    (MCI_OPEN_SHAREABLE | MCI_WAIT | MCI_OPEN_ELEMENT | MCI_OPEN_PLAYLIST | MCI_OPEN_MMIO)


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
  PMMDRV_OPEN_PARMS    pDrvOpenParms;            // Pointer to MMDRV_OPEN structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pDrvOpenParms  = (PMMDRV_OPEN_PARMS)pFuncBlock->pParam2;

  LOG_ENTER(1, "ulParam1 = 0x%lx, pszElementName = %p, [%s]",
            pFuncBlock->ulParam1, pDrvOpenParms->pszElementName,
            ulParam1 & MCI_OPEN_ELEMENT ? pDrvOpenParms->pszElementName : "");

  /*******************************************************/
  /* Validate that we have only valid flags              */
  /*******************************************************/
  if (ulParam1 & ~(MCIOPENVALIDFLAGS))
     LOG_RETURN(1, MCIERR_INVALID_FLAG);

  /* MCI_OPEN_PLAYLIST are not supported */
  if (ulParam1 & MCI_OPEN_PLAYLIST)
     LOG_RETURN(1, MCIERR_UNSUPPORTED_FLAG);

  /*******************************************************************/
  /* Allocate and initialize the instance structure                  */
  /*******************************************************************/
  if (!(pInstance = malloc(sizeof(INSTANCE))))
     ulrc = MCIERR_OUT_OF_MEMORY;
  else
     {
     memset(pInstance, 0, sizeof(INSTANCE));

     if (DosCreateMutexSem(NULL,&(pInstance->hmtxAccessSem),DC_SEM_SHARED,FALSE))
        ulrc = MCIERR_DRIVER_INTERNAL;

     if (!ulrc)
        {
        /*******************************************************************/
        /* Set up default values in instance structure.                    */
        /*  NOTE this is a minimal instance structure                      */
        /*******************************************************************/
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

        KMDECAUDIOINFO ai;

        ai.bps = KMDEC_BPS_S16;
        ai.channels = 2;
        ai.sampleRate = 44100;

        if (ulParam1 & (MCI_OPEN_ELEMENT | MCI_OPEN_MMIO))
           {
           const char *sf2 = getenv("KSOFTSEQ_SF2");
           struct stat st;

           if (!sf2 || stat(sf2, &st) == -1)
              sf2 = szDefaultSf2;

           LOG_MSG(1, "sf2 = [%s]", sf2);

           if (ulParam1 & MCI_OPEN_ELEMENT)
              {
              strcpy(pInstance->szFileName, pDrvOpenParms->pszElementName);

              pInstance->dec = kmdecOpen(pInstance->szFileName, sf2, &ai);
              }
           else /* if (ulParam1 & MCI_OPEN_MMIO) */
              {
              HMMIO fd = (HMMIO)pDrvOpenParms->pszElementName;

              pInstance->dec = kmdecOpenFdEx(fd, sf2, &ai, &io);
              }

           if (!pInstance->dec)
              {
              DosCloseMutexSem(pInstance->hmtxAccessSem);

              free(pInstance);

              LOG_RETURN(1, MCIERR_DRIVER_INTERNAL);
              }
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

           LOG_RETURN(1, MCIERR_DRIVER_INTERNAL);
           }

        kaiEnableSoftVolume(pInstance->hkai, TRUE);
        }
     }


  LOG_RETURN(1, ulrc);

}      /* end of MCIOpen */
