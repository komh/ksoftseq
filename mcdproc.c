/****************************************************************************
**
** mcdproc.c
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
/* SOURCE FILE NAME:  MCDPROC.C                                             */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCD TEMPLATE MAIN PROCEDURE                           */
/*                                                                          */
/* COPYRIGHT:  (c) IBM Corp. 1991 - 1993                                    */
/*                                                                          */
/* FUNCTION:  This file contains the main procedure for the MCD template.   */
/*                                                                          */
/* NOTES:  For specific streaming or MMIO samples see the CD audio and      */
/*         waveform audio samples in the toolkit.  See the programming      */
/*         reference and programming guide for more details on MCDs and     */
/*         MCI messages and structures.                                     */
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*       mciDriverEntry - performs component specific messages of this MCI  */
/*                        Driver.                                           */
/*                                                                          */
/****************************************************************************/
#define INCL_BASE
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES


#define INCL_MCIOS2                  // use the OS/2 like MMPM/2 headers

#include <os2.h>                     // OS2 defines.
#include <string.h>                  // string prototypes
#include <stdlib.h>                  // standard C functions
#include <os2me.h>                   // MME includes files.
#include "mcdtemp.h"                 // MCD Function Prototypes and typedefs

CHAR szLogFile[] = "x:\\MMOS2\\KSOFTSEQ.LOG";
CHAR szDefaultSf2[] = "x:\\MMOS2\\KSOFTSEQ.SF2";

int _CRT_init(void);
void _CRT_term(void);
void __ctordtorInit(void);
void __ctordtorTerm(void);

unsigned long _System _DLL_InitTerm(unsigned long hmod, unsigned long flag)
{
    switch (flag)
    {
    case 0: // Initialization
        if (_CRT_init() == -1)
            return 0;

        __ctordtorInit();

        if (kaiInit(KAIM_AUTO))
            return 0;

        ULONG ulBootDrive;
        DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                        &ulBootDrive, sizeof(ulBootDrive));

        ulBootDrive += 'A' - 1;
        szLogFile[0] = ulBootDrive;
        szDefaultSf2[0] = ulBootDrive;

        return 1;

    case 1: // Termination
        kaiDone();

        __ctordtorTerm();

        _CRT_term();

        return 1;
    }

  return 0;
}


/****************************************************************************/
/*                                                                          */
/* SUBROUTINE NAME:  mciDriverEntry                                         */
/*                                                                          */
/* DESCRIPTIVE NAME:  MCI Driver Entry                                      */
/*                                                                          */
/* FUNCTION:  Processes the MCI messages sent from MDM..                    */
/*                                                                          */
/* PARAMETERS:                                                              */
/*      PVOID  lpInstance -- Pointer to device handle.                      */
/*      USHORT usMessage  -- Command message.                               */
/*      ULONG  ulParam1   -- Flag for this message.                         */
/*      PVOID  pParam2    -- Pointer to data record structure.              */
/*      USHORT usUserParm -- User Parameter for mciDriverNotify.            */
/*                                                                          */
/* EXIT CODES:                                                              */
/*      MCIERR_SUCCESS    -- Action completed without error.                */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*            .                                                             */
/*                                                                          */
/****************************************************************************/

/* This MCI driver is compiled with optlink linkage in C-SET/2
   The following pragma allows other dll's etc to call in */

#ifndef __KLIBC__
#pragma linkage(mciDriverEntry, system )
#endif

ULONG APIENTRY mciDriverEntry(PVOID pInstance,
                              USHORT usMessage,
                              ULONG ulParam1,
                              PVOID pParam2,
                              USHORT usUserParm)


{
  ULONG                   ulrc;                // Return Code
  FUNCTION_PARM_BLOCK     ParamBlock;          // Encapsulate Parameters

  LOG_ENTER("usMessage = %d, ulParam1 = 0x%lx, usUserParm = %d",
            usMessage, ulParam1, usUserParm);

  /***********************************************/
  /* Copy the mciDriverEntry parameters to a     */
  /* structure which contains all of them.  This */
  /* simplifies calling functions etc. since we  */
  /* can pass one parameter rather than 5 or 6.  */
  /***********************************************/

  ParamBlock.usMessage    = usMessage;
  ParamBlock.pInstance    = (PINSTANCE)pInstance;
  ParamBlock.usUserParm   = usUserParm;
  ParamBlock.ulParam1     = ulParam1;
  ParamBlock.pParam2      = (PVOID)pParam2;

  if (usMessage != MCI_OPEN)
    DosRequestMutexSem(ParamBlock.pInstance->hmtxAccessSem, -2);

  /***********************************************/
  /* Switch based on the MCI message.            */
  /* For each message perform error checking and */
  /* then message processing.                    */
  /*                                             */
  /***********************************************/
  switch (usMessage)
    {

    case MCI_OPEN:
      ulrc = MCIOpen(&ParamBlock);
     break;

    case MCI_CLOSE:
      ulrc = MCIClose(&ParamBlock);
     break;

    case MCIDRV_SAVE:
      ulrc = MCIDRVSave(&ParamBlock);
     break;

    case MCIDRV_RESTORE:
      ulrc = MCIDRVRestore(&ParamBlock);
     break;

    case MCI_STATUS:
      ulrc = MCIStatus(&ParamBlock);
     break;

    case MCI_INFO:
      ulrc = MCIInfo(&ParamBlock);
     break;

    case MCI_GETDEVCAPS:
      ulrc = MCICaps(&ParamBlock);
     break;

    case MCI_LOAD:
      ulrc = MCILoad(&ParamBlock);
     break;

    case MCI_PAUSE:
      ulrc = MCIPause(&ParamBlock);
     break;

    case MCI_PLAY:
      ulrc = MCIPlay(&ParamBlock);
     break;

    case MCI_RESUME:
      ulrc = MCIResume(&ParamBlock);
     break;

    case MCI_SEEK:
      ulrc = MCISeek(&ParamBlock);
     break;

    case MCI_SET:
      ulrc = MCISet(&ParamBlock);
     break;

    case MCI_SET_CUEPOINT:
      ulrc = MCISetCuePoint(&ParamBlock);
     break;

    case MCI_SET_POSITION_ADVISE:
      ulrc = MCISetPositionAdvise(&ParamBlock);
     break;

    case MCI_STOP:
      ulrc = MCIStop(&ParamBlock);
     break;

    case MCI_ESCAPE:
    case MCI_SPIN:
    case MCI_STEP:
    case MCI_RECORD:
    case MCI_SAVE:
    case MCI_CUE:
    case MCI_UPDATE:
    case MCI_SET_SYNC_OFFSET:
    case MCI_MASTERAUDIO:
    case MCI_GETTOC:
    case MCI_DEVICESETTINGS:
    case MCI_CONNECTOR:
    case MCI_CONNECTORINFO:
    case MCI_CONNECTION:
    case MCI_CAPTURE:
    case MCI_FREEZE:
    case MCI_GETIMAGEBUFFER:
    case MCI_GETIMAGEPALETTE:
    case MCI_PUT:
    case MCI_REALIZE:
    case MCI_REWIND:
    case MCI_RESTORE:
    case MCI_SETIMAGEBUFFER:
    case MCI_SETIMAGEPALETTE:
    case MCI_UNFREEZE:
    case MCI_WHERE:
    case MCI_WINDOW:
    case MCI_DELETE:
    case MCI_CUT:
    case MCI_PASTE:
    case MCI_COPY:
    case MCI_REDO:
    case MCI_UNDO:
    default:
      ulrc = MCIERR_UNRECOGNIZED_COMMAND;
      break;


    }   /* Switch */

  if (usMessage != MCI_OPEN)
    DosReleaseMutexSem(ParamBlock.pInstance->hmtxAccessSem);

  /* process MCI_WAIT of MCI_PLAY here to avoid a dead lock by hmtxAccessSem */
  if (usMessage == MCI_PLAY && !ulrc && ulParam1 & MCI_WAIT)
    {
    while (kaiStatus(ParamBlock.pInstance->hkai) & KAIS_PLAYING)
      DosSleep(1);
    }

  if (usMessage == MCI_CLOSE && !ulrc)
    {
      DosCloseMutexSem(ParamBlock.pInstance->hmtxAccessSem);
      free(ParamBlock.pInstance);
    }

  LOG_RETURN(ulrc);    /* Return to MDM */

} /* mciDriverEntry */
