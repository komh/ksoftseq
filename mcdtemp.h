/****************************************************************************
**
** mcdtemp.h
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

/********************** START OF SPECIFICATIONS ****************************
*
*   SOURCE FILE NAME:   MCDTEMP.H
*
*   DESCRIPTIVE NAME: MCD Template defines and prototypes
*
*              Copyright (c) IBM Corporation  1993
*                        All Rights Reserved
*
*   FUNCTION: Component function Prototypes.
*
*****************************************************************************/

#include <kai.h>
#include <kmididec.h>

#include "klogger.h"

#define KSOFTSEQ_VERSION    "1.0.0"

#define    MAX_FILE_NAME                255


typedef struct {
    HWND    hwndCallback;
    USHORT  usUserParm;
} PLAYNOTIFY;

typedef struct {
    HWND    hwndCallback;
    ULONG   ulCuepoint;                 /* in ms */
    USHORT  usUserParm;
    BOOL    On;
    BOOL    Notified;
} CUENOTIFY;

#define MAX_CUE_POINTS  20

typedef struct {
    HWND    hwndCallback;
    ULONG   ulUnits;                    /* in ms */
    ULONG   ulNext;                     /* in ms */
    USHORT  usUserParm;
} ADVISENOTIFY;


/********************************************************************
*   This Structure defines the data items that are needed to be
*   maintained per MCI Device instance. The Instance structure
*   has a pointer to the Streaming Information Structure defined
*   above. Memory For This data structure is allocated dynamically
*   off a Global heap.
********************************************************************/

typedef struct _Instance {
    USHORT    usDeviceID;                /* Device ID            */
    USHORT    usDeviceType;              /* Device type number   */
    USHORT    usDeviceOrd;               /* Device ordinal       */
    HMTX      hmtxAccessSem;             /* instance access semphore */
    ULONG     ulSyncOffset;              /* Synchronizatn offst  */
    ULONG     ulVolume;                  /* Instance Volume      */
    ULONG     ulMasterVolume;            /* Master Volume        */
    ULONG     ulTimeFormat;              /* Current Time Format  */
    ULONG     ulSpeedFormat;             /* Current SpeedFormat  */
    ULONG     ulState;                   /* Current instance state */
    ULONG     ulMode;                    /* Current instance state */
    ULONG     ulCurrentPosition;         /* Current position     */
    ULONG     ulStartPosition;           /* Start position       */
    ULONG     ulEndPosition;             /* End   position       */
    BOOL      Active;                    /* True if instance is active */
    BOOL      Speaker;                   /* True if speaker should be on */
    BOOL      Headphone;                 /* True if headphone should be on */
    CHAR      szProductInfo[MAX_PRODINFO]; /* Textual product description    */
    CHAR      szInstallName[MAX_DEVICE_NAME]; /* Device install name            */
    CHAR      szDevParams[MAX_DEV_PARAMS];
    CHAR      szFileName[MAX_FILE_NAME];
    HKAI      hkai;
    ULONG     ulTolerance;
    ULONG     ulSavedStatus;
    PKMDEC    dec;
    PLAYNOTIFY playNotify;
    CUENOTIFY cueNotify[MAX_CUE_POINTS];
    ADVISENOTIFY adviseNotify;
    } INSTANCE;         /* Audio MCD MCI Instance Block */
typedef INSTANCE *PINSTANCE;


/*********************************************************************
* Function Parameter Block defines the parameters needed by each MCI
* Message processing function to process the message.
**********************************************************************/
typedef struct {
    USHORT      usMessage;         /*  The Message                       */
    ULONG       ulParam1;          /*  Flags for the Message             */
    PVOID       pParam2;           /*  Data for The Message              */
    USHORT      usUserParm;        /*  User Parameter Returned on Notify */
    ULONG       ulNotify;          /*  Notification flag                 */
    PINSTANCE   pInstance;         /*  Shall be Modified later           */
    }FUNCTION_PARM_BLOCK;          /*  Audio MCD Function Paramter Block */
typedef FUNCTION_PARM_BLOCK *PFUNCTION_PARM_BLOCK;


/***********************************************
* MCI Message Support Functions
***********************************************/
RC   MCIOpen          (FUNCTION_PARM_BLOCK *);// Open
RC   MCIOpenErr       (FUNCTION_PARM_BLOCK *);// Open Error
RC   MCIClose         (FUNCTION_PARM_BLOCK *);// Close
RC   MCICloseErr      (FUNCTION_PARM_BLOCK *);// Close Error
RC   MCIDRVSave       (FUNCTION_PARM_BLOCK *);// Save
RC   MCIDRVSaveErr    (FUNCTION_PARM_BLOCK *);// Save Error
RC   MCIDRVRestore    (FUNCTION_PARM_BLOCK *);// Restore
RC   MCIDRVRestoreErr (FUNCTION_PARM_BLOCK *);// Restore Error

/***********************************************/
/* List of valid instance states or modes      */
/***********************************************/
#define MCD_MODE_NOT_READY     MCI_MODE_NOT_READY
#define MCD_MODE_PAUSE         MCI_MODE_PAUSE
#define MCD_MODE_PLAY          MCI_MODE_PLAY
#define MCD_MODE_STOP          MCI_MODE_STOP
#define MCD_MODE_RECORD        MCI_MODE_RECORD
#define MCD_MODE_SEEK          MCI_MODE_SEEK
#define MCD_MODE_OPENING       0x0001000L
#define MCD_MODE_CLOSING       0x0002000L
#define MCD_MODE_LOADING       0x0003000L

/***********************************************/
/* MCD function Prototyes                      */
/***********************************************/
ULONG  EXPENTRY mciDriverEntry ( PVOID    pInstance,
                                 USHORT   usMessage,
                                 ULONG    ulParam1,
                                 PVOID     pParam2,
                                 USHORT   usUserParm);
RC    MCIOpen (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIOpenErr (FUNCTION_PARM_BLOCK *pFuncBlock);
VOID  GetINIInstallName(PINSTANCE pInstance);
VOID  GetDeviceInfo(PINSTANCE pInstance);
VOID  QMAudio(PINSTANCE pInstance);
RC    MCIStatus (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIStatusErr (FUNCTION_PARM_BLOCK *pFuncBlock);
ULONG ConvertTime(ULONG ulTime, ULONG ulCurrentFormat, ULONG ulNewFormat);
RC    MCIInfo   (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIInfoErr (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIDRVRestore (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIDRVRestoreErr (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIDRVSave (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIDRVSaveErr (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCICaps (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCILoad (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIPause (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIPlay (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIResume (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCISeek (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCISet (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCISetCuePoint (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCISetPositionAdvise (FUNCTION_PARM_BLOCK *pFuncBlock);
RC    MCIStop (FUNCTION_PARM_BLOCK *pFuncBlock);

/***********************************************/
/* Logging macros                              */
/***********************************************/

extern CHAR szLogFile[];
extern CHAR szDefaultSf2[];

#define LOG_ENTER(format, ...) \
        kloggerFile(szLogFile, "%s entered: " format, __func__, __VA_ARGS__)

#define LOG_MSG(format, ...) \
        kloggerFile(szLogFile, "%s: " format, __func__, __VA_ARGS__)

#define LOG_RETURN(rc) do { \
        kloggerFile(szLogFile, "%s returned, rc = %ld", __func__, (rc)); \
        return (rc); } while (0)
