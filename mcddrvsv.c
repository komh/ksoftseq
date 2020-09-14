/****************************************************************************
**
** mcddrvsv.c
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
/* SOURCE FILE NAME:  MCDDRVSV.C                                            */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCIDRV_SAVE MESSAGE HANDLER                           */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the MCIDRV_SAVE message.*/
/*            The MCIDRV_SAVE message tells us that MDM is save this device */
/*            instance (making inactive).  Save the state of the device in  */
/*            the instance structure                                        */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIDRVSave() - MCIDRV_SAVE message handler                         */
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
/* MCIDRV_SAVE valid flags                     */
/***********************************************/
#define MCIDRVSAVEVALIDFLAGS   MCI_WAIT


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIDRVSave                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCIDRV_SAVE message processor                         */
/*                                                                          */
/* FUNCTION:  Process the MCIDRV_SAVE message.  Make the instance inactive. */
/*                                                                          */
/* PARAMETERS:                                                              */
/*      FUNCTION_PARM_BLOCK  *pFuncBlock -- Pointer to function parameter   */
/*                                          block. This block is allocated  */
/*                                          by the main routine message     */
/*                                          router.                         */
/* EXIT CODES:                                                              */
/*      MCIERR_SUCCESS    -- Action completed without error.                */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*                                                                          */
/****************************************************************************/
RC MCIDRVSave (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_GENERIC_PARMS   pDrvSaveParms;            // Pointer to GENERIC structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pInstance      = pFuncBlock->pInstance;
  pDrvSaveParms     = (PMCI_GENERIC_PARMS)pFuncBlock->pParam2;

  LOG_ENTER("ulParam1 = 0x%lx", ulParam1);

  /*******************************************************/
  /* Validate that we have only valid flags              */
  /*******************************************************/
  if (ulParam1 & ~(MCIDRVSAVEVALIDFLAGS))
     LOG_RETURN(MCIERR_INVALID_FLAG);

  /*****************************************************/
  /* NOTE ----->>>                                     */
  /*  This is the basic function that should be        */
  /*  performed.  See the other samples in the toolkit */
  /*  for streaming and MMIO considerations            */
  /*****************************************************/
  pInstance->ulSavedStatus = kaiStatus(pInstance->hkai);
  if ((pInstance->ulSavedStatus & (KAIS_PLAYING | KAIS_PAUSED)) ==
      KAIS_PLAYING)
    kaiPause(pInstance->hkai);
  pInstance->Active = FALSE;

  /* make compiler happy */
  (void)pDrvSaveParms;


  LOG_RETURN(ulrc);

}      /* end of MCIDRVSave */
