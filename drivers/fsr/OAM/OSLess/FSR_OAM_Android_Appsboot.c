/**
 *   @mainpage   Flex Sector Remapper
 *
 *   @section Intro
 *       Flash Translation Layer for Flex-OneNAND and OneNAND
 *
 *    @section  Copyright
 *            COPYRIGHT. 2003-2009 SAMSUNG ELECTRONICS CO., LTD.
 *                            ALL RIGHTS RESERVED
 *
 *     Permission is hereby granted to licensees of Samsung Electronics
 *     Co., Ltd. products to use or abstract this computer program for the
 *     sole purpose of implementing a product based on Samsung
 *     Electronics Co., Ltd. products. No other rights to reproduce, use,
 *     or disseminate this computer program, whether in part or in whole,
 *     are granted.
 *
 *     Samsung Electronics Co., Ltd. makes no representation or warranties
 *     with respect to the performance of this computer program, and
 *     specifically disclaims any responsibility for any damages,
 *     special or consequential, connected with the use of this program.
 *
 *     @section Description
 *
 */

/**
 * @file      FSR_OAM_Android_Appsboot.c
 * @brief     This file contain the OS Adaptation Modules for msm720x appsboot of android platform
 * @author
 * @date
 * @remark
 * REVISION HISTORY
 *
 */
#if defined (IMAGE_APPS_PROC)
#if !defined (USE_APPS_WM)
// namohwang
//#define     OAM_USE_KXMUTEX
#endif
#endif

#if defined (FSR_OAM_ALLOC_SMALL_HEAP)
#define     LOCAL_MEM_SIZE      ((32 * 1024) / sizeof(UINT32))
#elif defined (FSR_OAM_ALLOC_LARGE_HEAP)
#define     LOCAL_MEM_SIZE      ((5 * 100 * 1024) / sizeof(UINT32))
#else
#define     LOCAL_MEM_SIZE      ((1 * 100 * 1024) / sizeof(UINT32))
#endif

/******************************************************************************/
/* Header file inclusions                                                     */
/******************************************************************************/
/* standard I/O include files */

/* Android platform have not standard libc */

#include "../../PAM/MSM7k/targtsncjnlya.h"

/* FSR include file */
#define     FSR_NO_INCLUDE_STL_HEADER
#include    "FSR.h"
#include    "FSR_SpinLock.h"

/*****************************************************************************/
/* Global variables definitions                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Local #defines                                                            */
/*****************************************************************************/
#define     FSR_OAM_CHK_HEAPUSAGE
#ifndef FSR_USE_DUAL_CORE
#define     FSR_USE_DUAL_CORE
#endif

#define SPIN_LOCK()         smem_flash_spin_lock(FSR_SMEM_SPINLOCK_BML, FSR_SMEM_SPINLOCK_BASEADDR)
#define SPIN_UNLOCK()       smem_flash_spin_unlock(FSR_SMEM_SPINLOCK_BML, FSR_SMEM_SPINLOCK_BASEADDR)

#define ACQUIRE_SM()        SPIN_LOCK()
#define RELEASE_SM()        SPIN_UNLOCK()
#define CREATE_SM()

/*****************************************************************************/
/* Local typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local constant definitions                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/
#if defined(FSR_USE_DUAL_CORE)
PRIVATE     UINT32          gnShMemBaseAddress[FSR_MAX_HEAP_MEM_CHUNKS] = {0,0};
PRIVATE     UINT32          gnShMemMaxSize[FSR_MAX_HEAP_MEM_CHUNKS]     = {0,0};
PRIVATE     UINT32          gnSMallocPtr[FSR_MAX_HEAP_MEM_CHUNKS]       = {0,0};
PRIVATE     UINT32          gnShMemResetPtr[FSR_MAX_HEAP_MEM_CHUNKS]    = {0,0};
#endif

#if defined(FSR_OAM_CHK_HEAPUSAGE)
PRIVATE UINT32 gnFSRHeapUsage    = 0;
PRIVATE UINT32 gnFSRNumOfMemReqs = 0;
#endif

#if defined(FSR_NW) || defined (FSR_NBL2)
PRIVATE UINT32  gaMemBuf[LOCAL_MEM_SIZE];
PRIVATE UINT32  gnMallocPtr = 0;
#endif

/*****************************************************************************/
/* Static function prototypes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* External variables definitions                                            */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*****************************************************************************/
/* External function prototypes                                              */
/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//extern void  UART_SendString            (char *string);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

/**
 * @brief           This function initializes OAM
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark          FSR_PAM_Init() should be called before FSR_OAM_Init() is called.
 *                  this function is called by FSR_BML_Init()
 */
PUBLIC INT32
FSR_OAM_Init(VOID)
{
#if defined(FSR_USE_DUAL_CORE)
    INT32       nPAMRe = FSR_PAM_SUCCESS;
    FsrVolParm  stPAM[FSR_MAX_VOLS];
#endif

    FSR_STACK_VAR;

    FSR_STACK_END;

    FSR_OAM_InitMemStat();

#if defined(FSR_USE_DUAL_CORE)
    nPAMRe  = FSR_PAM_GetPAParm(stPAM);
    if (nPAMRe != FSR_PAM_SUCCESS)
    {
        return FSR_OAM_CRITICAL_ERROR;
    }

    gnShMemBaseAddress[0]   = stPAM[0].nSharedMemoryBase;
    gnShMemBaseAddress[1]   = stPAM[1].nSharedMemoryBase;

    gnShMemMaxSize[0]       = stPAM[0].nSharedMemorySize;
    gnShMemMaxSize[1]       = stPAM[1].nSharedMemorySize;
#endif

    return FSR_OAM_SUCCESS;
}

/**
 *
 * @brief           This function initializes  memory allocation statistics.
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID
FSR_OAM_InitMemStat(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    gnFSRHeapUsage    = 0;
    gnFSRNumOfMemReqs = 0;
}

/**
 * @brief           This function gets memory statistics
 *
 * @param[out]      pnHeapUsage    : the heap usage
 * @param[out]      pnNumOfMemReqs : the number of memory allocation requests
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          this function is used for getting memory usages
 *
 */
PUBLIC VOID
FSR_OAM_GetMemStat(UINT32  *pnHeapUsage,
                   UINT32  *pnNumOfMemReqs)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    *pnHeapUsage    = gnFSRHeapUsage;
    *pnNumOfMemReqs = gnFSRNumOfMemReqs;

    FSR_RTL_PRINT((TEXT("[OAM:   ] HeapUsage : %d / NumOfMemReqs : %d\r\n"), gnFSRHeapUsage, gnFSRNumOfMemReqs));
}

/**
 * @brief           This function sets malloc reset pointer
 *
 * @param[in]       nMemChunkID : memory chunck ID (0, 1)
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          internal debug purpose ONLY
 *
 */
PUBLIC VOID
FSR_OAM_SetMResetPoint(UINT32  nMemChunkID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* <Caution>
     * This API sets only the shared memory pointer
     */
    gnShMemResetPtr[nMemChunkID]    = gnSMallocPtr[nMemChunkID];
}

/**
 * @brief           This function resets malloc pointer
 *
 * @param[in]       nMemChunkID : memory chunck ID (0, 1)
 * @param[in]       bReset      : if TRUE32, Malloc pointer is set as 0
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          If there is no malloc/free API, use this API instead of FSR_OAM_Free()
 *                  to free all allocated memory. internal debug purpose ONLY
 *
 */
PUBLIC VOID
FSR_OAM_ResetMalloc(UINT32  nMemChunkID,
                    BOOL32  bReset)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* <Caution>
     * This API resets only the shared memory pointer
     */
    if (bReset == TRUE32)
    {
        gnSMallocPtr[nMemChunkID]   = 0;
    }
    else /* bReset == FALSE32 */
    {
        gnSMallocPtr[nMemChunkID]   = gnShMemResetPtr[nMemChunkID];
    }
}

/**
 * @brief           This function allocates memory
 *
 * @param[in]       nSize    : Size to be allocated
 *
 * @return          Pointer of allocated memory
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID *
FSR_OAM_Malloc(UINT32 nSize)
{
#if defined(FSR_NW) || defined (FSR_NBL2)
    UINT32  nAlignSize;

    FSR_STACK_VAR;

    FSR_STACK_END;

#if defined (FSR_OAM_CHK_HEAPUSAGE)
    gnFSRHeapUsage += nSize;
    gnFSRNumOfMemReqs++;
#endif

    nAlignSize = nSize / sizeof(UINT32);
    if (nSize % sizeof(UINT32))
    {
        nAlignSize++;
    }

    gnMallocPtr += nAlignSize;
    if (gnMallocPtr > LOCAL_MEM_SIZE)
    {
        return NULL;
    }

    return (VOID *) &(gaMemBuf[gnMallocPtr - nAlignSize]);
#else
    VOID *pMemPtr;

    pMemPtr = (VOID *) malloc(nSize);
    return pMemPtr;
#endif
}

/**
 * @brief           This function allocates memory
 *
 * @param[in]       nMemChunkID : Memory chunk ID (0, 1)
 * @param[in]       nSize       : Size to be allocated
 * @param[in]       nMemType    : Memory type to be allocated
 *
 * @return          Pointer of allocated memory
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID*
FSR_OAM_MallocExt(UINT32    nMemChunkID,
                  UINT32    nSize,
                  UINT32    nMemType)
{
#if defined(FSR_USE_DUAL_CORE)
    UINT32  nAlignSize;

    if (nMemType == FSR_OAM_LOCAL_MEM)
    {
        return FSR_OAM_Malloc(nSize);
    }
    else /* nMemType == FSR_OAM_SHARED_MEM */
    {
        nAlignSize = nSize / sizeof(UINT32);
        if (nSize % sizeof(UINT32))
        {
            nAlignSize++;
        }

        gnSMallocPtr[nMemChunkID] += nAlignSize;
        if (gnSMallocPtr[nMemChunkID] > gnShMemMaxSize[nMemChunkID])
        {
            return NULL;
        }

        return (VOID *) (gnShMemBaseAddress[nMemChunkID] + (gnSMallocPtr[nMemChunkID] - nAlignSize) * 4);
    }
#else
    return NULL;
#endif
}

/**
 * @brief           This function frees memory
 *
 * @param[in]       nMemChunkID : Memory chunk ID (0, 1)
 * @param[in]      *pMem        : Pointer to be free
 * @param[in]       nMemType    : Memory type to be free
 *
 * @return         none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID
FSR_OAM_FreeExt(UINT32      nMemChunkID,
                VOID       *pMem,
                UINT32      nMemType)
{
#if defined(FSR_USE_DUAL_CORE)
    if (nMemType == FSR_OAM_LOCAL_MEM)
    {
        FSR_OAM_Free(pMem);
    }
    else
    {
        FSR_OAM_ResetMalloc(nMemChunkID, FALSE32);
    }
#endif
}

/**
 * @brief           This function frees memory
 *
 * @param[in]      *pMem : Pointer to be free
 *
 * @return         none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID
FSR_OAM_Free(VOID  *pMem)
{
    FSR_STACK_VAR;

    FSR_STACK_END;
#if defined(FSR_NW) || defined (FSR_NBL2)
    /* no action */
#else
    free(pMem);
#endif
}

/**
 * @brief           This function creates semaphore object.
 *
 * @param[out]     *pHandle : Handle of semaphore
 * @param[in]       nLayer  : 0 : FSR_OAM_SM_TYPE_BDD
 *                            0 : FSR_OAM_SM_TYPE_STL
 *                            1 : FSR_OAM_SM_TYPE_BML
 *                            2 : FSR_OAM_SM_TYPE_LLD
 *
 * @return          TRUE32   : this function creates semaphore successfully
 * @return          FALSE32  : fail
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          An initial count of semaphore object is 1.
 *                  An maximum count of semaphore object is 1.
 *
 */
PUBLIC BOOL32
FSR_OAM_CreateSM(SM32   *pHandle,
                 UINT32  nLayer)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    CREATE_SM();

    return TRUE32;
}

/**
 * @brief           This function destroys semaphore.
 *
 * @param[in]       nHandle : Handle of semaphore to be destroyed
 * @param[in]       nLayer  : 0 : FSR_OAM_SM_TYPE_BDD
 *                            0 : FSR_OAM_SM_TYPE_STL
 *                            1 : FSR_OAM_SM_TYPE_BML
 *                            2 : FSR_OAM_SM_TYPE_LLD
 *
 * @return          TRUE32   : this function destroys semaphore successfully
 * @return          FALSE32  : fail
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC BOOL32
FSR_OAM_DestroySM(SM32        nHandle,
                  UINT32      nLayer)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* NO semaphore APIs in OSLess environemts */

    return TRUE32;
}

/**
 * @brief          This function acquires semaphore.
 *
 * @param[in]       nHandle : Handle of semaphore to be acquired
 * @param[in]       nLayer  : 0 : FSR_OAM_SM_TYPE_BDD
 *                            0 : FSR_OAM_SM_TYPE_STL
 *                            1 : FSR_OAM_SM_TYPE_BML
 *                            2 : FSR_OAM_SM_TYPE_LLD
 *
 * @return         TRUE32   : this function acquires semaphore successfully
 * @return         FALSE32  : fail
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC BOOL32
FSR_OAM_AcquireSM(SM32        nHandle,
                  UINT32      nLayer)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    ACQUIRE_SM();

    return TRUE32;
}

/**
 * @brief           This function releases semaphore.
 *
 * @param[in]       nHandle : Handle of semaphore to be released
 * @param[in]       nLayer  : 0 : FSR_OAM_SM_TYPE_BDD
 *                            0 : FSR_OAM_SM_TYPE_STL
 *                            1 : FSR_OAM_SM_TYPE_BML
 *                            2 : FSR_OAM_SM_TYPE_LLD
 *
 * @return          TRUE32   : this function releases semaphore successfully
 * @return          FALSE32  : fail
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC BOOL32
FSR_OAM_ReleaseSM(SM32        nHandle,
                  UINT32      nLayer)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    RELEASE_SM();

    return TRUE32;
}

/**
 * @brief           This function prints debug message
 *
 * @param[in]       *pSrc        Source data allocated Pointer 
 *                  *pDst        Destination array Pointer to be compared
 *                   nLen        length to be compared  
 *
 * @return          
 *
 * @author          
 * @version         1.0.0
 * @remark          This function is called by function that wants to compare source
 *                  buffer and destination buffer. 
 *
 */
INT32
FSR_OAM_Memcmp(VOID  *pSrc, VOID  *pDst, UINT32 nLen)
{
#if defined(OAM_USE_STDLIB)
        return memcmp(pSrc, pDst, nLen);
#else /* OAM_USE_STDLIB */
        UINT8 *pS1 = (UINT8 *)pSrc;
        UINT8 *pD1 = (UINT8 *)pDst;

        while (nLen--)
        {
                if (*pS1 != *pD1)
                        return (*pS1 - *pD1);
        pS1++;
        pD1++;
        }

        return 0;
#endif /* OAM_USE_STDLIB */
}

/**
 * @brief           This function prints debug message
 *
 * @param[in]       *pDst        Destination array Pointer to be compared
 *                   nData       Value to be setted  
 *                   nLen        length to be compared  
 *
 * @return          
 *
 * @author          
 * @version         1.0.0
 * @remark          This function is called by function that wants to compare source
 *                  buffer and destination buffer. 
 *
 */
VOID
FSR_OAM_Memset(VOID *pDst,UINT8 nV,UINT32 nLen)
{
#if defined(OAM_USE_STDLIB)
        memset(pDst, nV, nLen);
#else /* OAM_USE_STDLIB */
        register UINT8  *pD8;
        register UINT32 *pD32;
        register UINT8  nV8 = nV;
        register UINT32 nV32 = (UINT32)( nV << 24 | nV << 16 | nV << 8 | nV );
        register INT32  nL = (INT32)nLen;
        register UINT32 nCnt;

        pD8 = (UINT8*)pDst;

        while ( (INT32)pDst % sizeof(UINT32) )
        {
                *pD8++ = nV8;
                nL--;

                if( nL <= 0 )
                    return;
        }

        pD32 = (UINT32*)pD8;
        for (nCnt = 0; nCnt <(INT32)(nL / sizeof(UINT32)); nCnt++)
                *pD32++ = nV32;

        pD8 = (UINT8*)pD32;
        while( nL % sizeof(UINT32) )
        {
                *pD8++ = nV8;
                nL--;
        }

	return;
#endif /* OAM_USE_STDLIB */
}

/**
 * @brief           This function prints debug message
 *
 * @param[in]       *pDst        Destination array Pointer to be compared
 *                  *pSrc        Source data allocated Pointer 
 *                   nLen        length to be compared  
 *
 * @return          
 *
 * @author          
 * @version         1.0.0
 * @remark          This function is called by function that wants to compare source
 *                  buffer and destination buffer. 
 *
 */
VOID
FSR_OAM_Memcpy(VOID *pDst,VOID *pSrc,UINT32 nLen)
{
#if defined(OAM_USE_STDLIB)
        memcpy(pDst, pSrc, nLen);
#else /* OAM_USE_STDLIB */
        register INT32  nCnt;
        register UINT8  *pD8, *pS8;
        register INT32  nL = nLen;
        register UINT32 *pD32, *pS32;

        pD8 = (UINT8*)pDst;
        pS8 = (UINT8*)pSrc;

        if ( ((INT32)pD8 % sizeof(UINT32)) == ((INT32)pS8 % sizeof(UINT32)) )
        {
                while ( (INT32)pD8 % sizeof(UINT32) )
                {
                        *pD8++ = *pS8++;
                        nL--;

                        if( nL <= 0 )
                            return;
                }

                pD32 = (UINT32*)pD8;
                pS32 = (UINT32*)pS8;

                for (nCnt = 0; nCnt <(INT32)(nL / sizeof(UINT32)); nCnt++)
                        *pD32++ = *pS32++;

                pD8 = (UINT8*)pD32;
                pS8 = (UINT8*)pS32;

                while( nL % sizeof(UINT32) )
                {
                        *pD8++ = *pS8++;
                        nL--;
                }
        }
        else
        {
                for( nCnt = 0; nCnt < nL; nCnt++)
                        *pD8++ = *pS8++;
        }
#endif /* OAM_USE_STDLIB */
}

/**
 * @brief           This function prints debug message
 *
 * @param[in]       *pFmt : NULL-terminated string to be printed
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          This function is used when OS can not support debug print API
 *
 */
PUBLIC VOID
FSR_OAM_DbgMsg(VOID  *pFmt, ...)
{
/*
    DO NOT use     FSR_STACK_VAR/FSR_STACK_END macro
*/
}

/**
 * @brief           This function gets virtual address for NAND device physical address
 *
 * @param[in]       nPAddr : physical address of NAND device
 *
 * @return          Virtual address of NAND device
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC UINT32
FSR_OAM_Pa2Va(UINT32 nPAddr)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    return nPAddr;
}

/**
 * @brief           This function waits N msec
 *
 * @param[in]       nNMSec : msec time for waiting
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID
FSR_OAM_WaitNMSec(UINT32 nNMSec)
{
    FSR_STACK_VAR;

    FSR_STACK_END;
}

/**
 * @brief           This function is called in _IsROPartition function
 *
 * @return          TRUE32  : lock mechanism is used
 * @return          FALSE32 : lock mechanism isn't used
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC BOOL32
FSR_OAM_GetROLockFlag(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

#if defined(FSR_OAM_NO_USE_LOCK_MECHANISM)
    return  FALSE32;
#else
    return  TRUE32;
#endif
}

/**
 * @brief           This function initializes the specified logical interrupt.
 *
 * @param[in]       nLogIntId : logical interrupt ID
 *
 * @return          FSR_OAM_SUCCESS
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC INT32
FSR_OAM_InitInt(UINT32 nLogIntId)
{
    INT32       nRe = FSR_OAM_SUCCESS;

    FSR_STACK_VAR;

    FSR_STACK_END;

    switch (nLogIntId)
    {
        case FSR_INT_ID_NAND_0:

            /* register ISR for the logical interrupt */

            /* Clear and Disable the logical interrupt */
            FSR_OAM_ClrNDisableInt(nLogIntId);

            break;

        default:
            break;
    }

    return nRe;
}

/**
 * @brief           This function initializes the specified logical interrupt.
 *
 * @param[in]       nLogIntId : logical interrupt ID
 *
 * @return          FSR_OAM_SUCCESS
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC INT32
FSR_OAM_DeinitInt(UINT32 nLogIntId)
{
    INT32       nRe = FSR_OAM_SUCCESS;

    FSR_STACK_VAR;

    FSR_STACK_END;

    switch (nLogIntId)
    {
        case FSR_INT_ID_NAND_0:

            /* unregister ISR for the logical interrupt */

            /* Clear and Disable the logical interrupt */
            FSR_OAM_ClrNDisableInt(nLogIntId);

            break;

        default:
            break;
    }

    return nRe;
}

/**
 * @brief           This function clears/disables the specified interrupt.
 *
 * @param[in]       nLogIntId : logical interrupt ID
 *
 * @return          FSR_OAM_SUCCESS
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC INT32
FSR_OAM_ClrNDisableInt(UINT32  nLogIntId)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    switch (nLogIntId)
    {
        case FSR_INT_ID_NAND_0:
            break;

        default:
            break;
    }

    return FSR_OAM_SUCCESS;
}

/**
 * @brief           This function clears/enables the specified interrupt.
 *
 * @param[in]       nLogIntId : logical interrupt ID
 *
 * @return          FSR_OAM_SUCCESS
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC INT32
FSR_OAM_ClrNEnableInt(UINT32  nLogIntId)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    switch (nLogIntId)
    {
        case FSR_INT_ID_NAND_0:
            break;

        default:
            break;
    }

    return FSR_OAM_SUCCESS;
}

/**
 * @brief           This function creates the event.
 *
 * @param[out]      *pHandle : event handle
 *
 * @return          TRUE32 or FALSE32
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC BOOL32
FSR_OAM_CreateEvent(UINT32    *pHandle)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* not implemented */

    return TRUE32;
}

/**
 * @brief           This function deletes the event.
 *
 * @param[in]       nHandle : event handle
 *
 * @return          TRUE32 or FALSE32
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC BOOL32
FSR_OAM_DeleteEvent(UINT32     nHandle)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* not implemented */

    return TRUE32;
}

/**
 * @brief           This function sends the event.
 *
 * @param[in]       nHandle : event handle
 *
 * @return          TRUE32 or FALSE32
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC BOOL32
FSR_OAM_SendEvent(UINT32     nHandle)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* not implemented */

    return TRUE32;
}

/**
 * @brief           This function receives the event.
 *
 * @param[in]       nHandle : event handle
 *
 * @return          TRUE32 or FALSE32
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark          this function is used to support non-blocking I/O feature of FSR
 *
 */
PUBLIC BOOL32
FSR_OAM_ReceiveEvent(UINT32     nHandle)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* not implemented */

    return TRUE32;
}

/**
 * @brief           This function starts timer
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark
 *
 */
PUBLIC VOID
FSR_OAM_StartTimer(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;
}

/**
 * @brief           This function stops timer
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark
 *
 */
PUBLIC VOID
FSR_OAM_StopTimer(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;
}

/**
 * @brief           This function get the elapsed time (usec)
 *
 * @return          the elapsed time (usec)
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @remark          if this function isn't implemented, it should return 0.
 *
 */
PUBLIC UINT32
FSR_OAM_GetElapsedTime(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    return (UINT32) (0);
}

/**
 * @brief           This function initializes OMAP2420 DMA channel 1
 *
 * @return          FSR_OAM_SUCCESS;
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark          FSR_OAM_InitDMA() is called after FSR_OAM_Init() is called in FSR_BML_Init()
 */
PUBLIC INT32
FSR_OAM_InitDMA(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    return FSR_OAM_SUCCESS;
}


/**
 * @brief           This function do read operation by DMA
 *
 * @param[in]       nVirDstAddr : virtual destination address
 * @param[in]       nVirSrcAddr : virtual source address
 * @param[in]       nSize       : size to be invalidated
 *
 * @return          FSR_OAM_SUCCESS
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC INT32
FSR_OAM_ReadDMA(UINT32     nVirDstAddr,
                UINT32     nVirSrcAddr,
                UINT32     nSize)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    return FSR_OAM_SUCCESS;
}

/**
 * @brief           This function do write operation by DMA
 *
 * @param[in]       nVirDstAddr : virtual destination address
 * @param[in]       nVirSrcAddr : virtual source address
 * @param[in]       nSize       : size to be invalidated
 *
 * @return          FSR_OAM_SUCCESS
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC INT32
FSR_OAM_WriteDMA(UINT32     nVirDstAddr,
                 UINT32     nVirSrcAddr,
                 UINT32     nSize)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    return FSR_OAM_SUCCESS;
}

/**
 * @brief           This function initializes shared memory flag
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 * @remark
 */
PUBLIC VOID
FSR_OAM_InitSharedMemory(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

   FSR_OAM_MEMSET((void*)gnShMemBaseAddress[0], 0x00,  gnShMemMaxSize[0] );
    /* not implemented */
}

