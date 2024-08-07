/****************************************************************************
**
** mcdclose.c
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
/* SOURCE FILE NAME:  MCDCLOSE.C                                            */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_CLOSE MESSAGE HANDLER                             */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the mci_close message.  */
/*            the device is instance is closed and all resources associated */
/*            with the device instance are freed.                           */
/*            See the programming reference and programming examples for    */
/*            more details on the MCI_CLOSE message.                        */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIClose() - MCI_CLOSE message handler                             */
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
/* MCI_CLOSE valid flags                       */
/***********************************************/
#define MCICLOSEVALIDFLAGS    (MCI_WAIT | MCI_NOTIFY | MCI_CLOSE_EXIT)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIClose                                               */
/*                                                                          */
/* DESCRIPTIVE NAME:  Close an instance.                                    */
/*                                                                          */
/* FUNCTION:  Close and free the given instance.                            */
/*                                                                          */
/* PARAMETERS:                                                              */
/*      FUNCTION_PARM_BLOCK *pFuncBlock - Pointer to call function block    */
/*                                                                          */
/* EXIT CODES:                                                              */
/*      MCIERR_SUCCESS    -- Action completed without error.                */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*                                                                          */
/****************************************************************************/
RC MCIClose (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_GENERIC_PARMS   pCloseParms;              // Pointer to GENERIC structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pInstance      = pFuncBlock->pInstance;
  pCloseParms    = (PMCI_GENERIC_PARMS)pFuncBlock->pParam2;

  LOG_ENTER(++pInstance->ulDepth, "ulParam1 = 0x%lx", ulParam1);

  /*******************************************************/
  /* Validate that we have only valid flags              */
  /*******************************************************/
  if (ulParam1 & ~(MCICLOSEVALIDFLAGS))
     LOG_RETURN(pInstance->ulDepth--, MCIERR_INVALID_FLAG);

  /*****************************************************/
  /* NOTE ----->>>                                     */
  /*  This is the basic function that should be        */
  /*  performed.  See the other samples in the toolkit */
  /*  for streaming and MMIO considerations            */
  /*****************************************************/
  pInstance->AvoidDeadLock = TRUE;

  kaiClose(pInstance->hkai);

  pInstance->AvoidDeadLock = FALSE;

  kmdecClose(pInstance->dec);

  /***************************************************************/
  /* Send back a notification if the notify flag was on          */
  /***************************************************************/
  if (ulParam1 & MCI_NOTIFY)
     ulrc = mdmDriverNotify (pInstance->usDeviceID,
                             pCloseParms->hwndCallback,
                             MM_MCINOTIFY,
                             pFuncBlock->usUserParm,
                             MAKEULONG (MCI_CLOSE, MCI_NOTIFY_SUCCESSFUL));


  LOG_RETURN(pInstance->ulDepth--, ulrc);

}      /* end of MCIClose */
