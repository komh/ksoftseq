/****************************************************************************
**
** mcddrvrt.c
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
/* SOURCE FILE NAME:  MCDDRVRT.C                                            */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCIDRV_RESTORE MESSAGE HANDLER                        */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCIDRV_RESTORE      */
/*            message.  This message tells us to make this device instance  */
/*            active again.  Restore the device based on the saved state    */
/*            information in the instance structure.                        */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIDRVRestore() - MCIDRV_RESTORE message handler                   */
/****************************************************************************/
#define INCL_BASE                    // Base OS2 functions
#define INCL_DOSSEMAPHORES           // OS2 Semaphore function
#define INCL_MCIOS2                  // use the OS/2 like MMPM/2 headers

#include <os2.h>                     // OS2 defines.
#include <string.h>                  // string prototypes
#include <stdlib.h>                  // standard C functions
#include <os2me.h>                   // MME includes files.
#include "mcdtemp.h"                 // MCD Function Prototypes and typedefs

/***********************************************/
/* MCIDRV_RESTORE valid flags                  */
/***********************************************/
#define MCIDRVRESTOREVALIDFLAGS    (MCI_WAIT | MCI_OPEN_SHAREABLE | MCI_EXCLUSIVE)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIDRVRestore                                          */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCIDRV_RESTORE message handler.                       */
/*                                                                          */
/* FUNCTION:  Query current master audio settings, and make this instance   */
/*            active.                                                       */
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
RC MCIDRVRestore (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_GENERIC_PARMS   pDrvRestoreParms;         // Pointer to GENERIC structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pInstance      = pFuncBlock->pInstance;
  pDrvRestoreParms = (PMCI_GENERIC_PARMS)pFuncBlock->pParam2;

  LOG_ENTER(++pInstance->ulDepth, "ulParam1 = 0x%lx", ulParam1);

  /*******************************************************/
  /* Validate that we have only valid flags              */
  /*******************************************************/
  if (ulParam1 & ~(MCIDRVRESTOREVALIDFLAGS))
     LOG_RETURN(pInstance->ulDepth--, MCIERR_INVALID_FLAG);

  /*****************************************************/
  /* NOTE ----->>>                                     */
  /*  This is the basic function that should be        */
  /*  performed.  See the other samples in the toolkit */
  /*  for streaming and MMIO considerations            */
  /*****************************************************/
  pInstance->Active = TRUE;                           // Set active to TRUE
  QMAudio(pInstance);                                 // Get master audio settings
  if ((pInstance->ulSavedStatus & (KAIS_PLAYING | KAIS_PAUSED)) ==
      KAIS_PLAYING)
     kaiResume(pInstance->hkai);

  /* clear ulSavedStatus for MCIDRV_RESTORE to an active instance */
  pInstance->ulSavedStatus = 0;

  /* make compiler happy */
  (void)pDrvRestoreParms;


  LOG_RETURN(pInstance->ulDepth--, ulrc);

}      /* end of Open */
