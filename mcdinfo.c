/****************************************************************************
**
** mcdinfo.c
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
/* SOURCE FILE NAME:  MCDINFO.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_INFO MESSAGE HANDLER                              */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains routines to handle the mci_info message.   */
/*            The two items shown here are infor file name and info product */
/*            See the programming reference for details of the info message.*/
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       MCIInfo() - MCI_INFO message handler                               */
/*       MCIInfoErr() - Error handler for MCI_INFO message                  */
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
/* MCI_INFO valid flags                        */
/***********************************************/
#define MCIINFOVALIDFLAGS    (MCI_WAIT | MCI_NOTIFY | MCD_INFO_FLAGS)

#define MCD_INFO_FLAGS       (MCI_INFO_PRODUCT | MCI_INFO_FILE)


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIInfo                                                */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_INFO message processor                            */
/*                                                                          */
/* FUNCTION:  Process the MCI_INFO message.                                 */
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
RC MCIInfo   (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_INFO_PARMS      pInfoParms;               // Pointer to info structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pInstance      = pFuncBlock->pInstance;
  pInfoParms     = (PMCI_INFO_PARMS)pFuncBlock->pParam2;

  LOG_ENTER("ulParam1 = 0x%lx", ulParam1);


  switch (ulParam1 & MCD_INFO_FLAGS)
    {
    /**********************************************************/
    /* Return a description of the hardware this device       */
    /*   represents                                           */
    /**********************************************************/
    case MCI_INFO_PRODUCT:
     if (pInfoParms->ulRetSize < MAX_PRODINFO)
        strncpy(pInfoParms->pszReturn, pInstance->szProductInfo, pInfoParms->ulRetSize);
     else
        strncpy(pInfoParms->pszReturn, pInstance->szProductInfo, MAX_PRODINFO);
     break;

    /**********************************************************/
    /* Return the file name associated with this device       */
    /*   instance                                             */
    /**********************************************************/
    case MCI_INFO_FILE:
     if (pInfoParms->ulRetSize < MAX_FILE_NAME)
        strncpy(pInfoParms->pszReturn, pInstance->szFileName, pInfoParms->ulRetSize);
     else
        strncpy(pInfoParms->pszReturn, pInstance->szFileName, MAX_FILE_NAME);
     break;

    default:
      ulrc = MCIERR_UNSUPPORTED_FLAG;
      break;


    }   /* Switch */

  /***************************************************************/
  /* Send back a notification if the notify flag was on          */
  /*   and no error                                              */
  /***************************************************************/
  if ((ulParam1 & MCI_NOTIFY) & (!ulrc))
     mdmDriverNotify (pInstance->usDeviceID,
                      pInfoParms->hwndCallback,
                      MM_MCINOTIFY,
                      pFuncBlock->usUserParm,
                      MAKEULONG (MCI_INFO, MCI_NOTIFY_SUCCESSFUL));


  LOG_RETURN(ulrc);

}      /* end of MCIInfo */


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  MCIInfoErr                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI_INFO message error processor                      */
/*                                                                          */
/* FUNCTION:  Process the MCI_INFO message for errors.                      */
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
RC MCIInfoErr (FUNCTION_PARM_BLOCK *pFuncBlock)
{
  ULONG                ulrc = MCIERR_SUCCESS;    // Propogated Error Code
  ULONG                ulParam1;                 // Message flags
  PMCI_INFO_PARMS      pInfoParms;               // Pointer to info structure
  PINSTANCE            pInstance;                // Pointer to instance

  /*****************************************************/
  /* dereference the values from pFuncBlock            */
  /*****************************************************/
  ulParam1       = pFuncBlock->ulParam1;
  pInstance      = pFuncBlock->pInstance;
  pInfoParms     = (PMCI_INFO_PARMS)pFuncBlock->pParam2;

  LOG_ENTER("ulParam1 = 0x%lx", ulParam1);

  /*******************************************************/
  /* Validate that we have only valid flags              */
  /*******************************************************/
  if (ulParam1 & ~(MCIINFOVALIDFLAGS))
     LOG_RETURN(MCIERR_INVALID_FLAG);

  /* make compiler happy */
  (void)pInstance;
  (void)pInfoParms;


  LOG_RETURN(ulrc);

}      /* end of MCIInfoErr */


