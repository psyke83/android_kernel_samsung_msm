/*
   %COPYRIGHT%
 */

/**
 * @version     %VERSION%
 * @file        drivers/tfsr/OAM/FSR_OAM_OSLess.c
 * @brief       This file contain the OS Adaptation Modules for NO OS
 *
 */

/* standard I/O include files */
#ifndef	FSR_UBOOT
#include    <stdio.h>
#include    <stdarg.h>
#if defined(OAM_USE_STDLIB)
#include    <string.h>
#endif /* OAM_USE_STDLIB */
#else
#include	<stdarg.h>
#endif /* FSR_UBOOT */
/* FSR include file */
#define     FSR_NO_INCLUDE_STL_HEADER
#include    "FSR.h"
#include    "../../../board/omap2430.h"
#include    "../../../board/2430board.h"
#include    "../../../board/mux.h"
/*****************************************************************************/
/* Global variables definitions                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Local #defines                                                            */
/*****************************************************************************/
#define     FSR_OAM_CHK_HEAPUSAGE
//#define     FSR_OAM_ALLOC_MEDIUM_HEAP

/* macros for interrupt */
#define     GPIO_8_OFFSET               (8)
#define     GPIO_72                     (1 << GPIO_8_OFFSET)     // gpio 72 -> gpio 8 (64+8)

#if defined(FSR_OAM_ALLOC_SMALL_HEAP)
    #define     LOCAL_MEM_SIZE      ((32 * 1024) / sizeof(UINT32))
#elif defined(FSR_OAM_ALLOC_MEDIUM_HEAP)
    #define     LOCAL_MEM_SIZE      ((5 * 100 * 1024) / sizeof(UINT32))
#else
//    #define     LOCAL_MEM_SIZE      ((5 * 1024 * 1024) / sizeof(UINT32))
    #define     LOCAL_MEM_SIZE      ((3 * 1024 * 1024) / sizeof(UINT32))
//    #define     LOCAL_MEM_SIZE      ((2 * 1024 * 1024) / sizeof(UINT32))
#endif

#define     ARM_GET_MVA(MVA, ClineSize)      (MVA & ~(ClineSize - 1))

/*****************************************************************************/
/* Local typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local constant definitions                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/
PRIVATE     UINT32  gnCacheLineSize       = 0;

#if defined(FSR_USE_DUAL_CORE)
PRIVATE     UINT32          gnShMemBaseAddress[FSR_MAX_VOLS]    = {0x01FFA000,0};
PRIVATE     UINT32          gnShMemMaxSize[FSR_MAX_VOLS]        = {0x5000,0};
PRIVATE     UINT32          gnSMallocPtr[FSR_MAX_VOLS]          = {0,0};

#define SHARED_MEMORY_RESET     (0x2)
#endif

#if defined(FSR_OAM_CHK_HEAPUSAGE)
PRIVATE UINT32 gnFSRHeapUsage    = 0;
PRIVATE UINT32 gnFSRNumOfMemReqs = 0;
#endif

PRIVATE UINT32  gaMemBuf[LOCAL_MEM_SIZE];
PRIVATE UINT32  gnMallocPtr                     = 0;
PRIVATE UINT32  gnMallocResetPtr                = 0;
PRIVATE BOOL32  gbEnableMMU                     = FALSE32;

#define is_digit(c)     ((c) >= '0' && (c) <= '9')
#ifndef FSR_UBOOT
#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */

#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })
#endif /* FSR_UBOOT */

/*****************************************************************************/
/* Static function prototypes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* External variables definitions                                            */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern float gnDiffTime;

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*****************************************************************************/
/* External function prototypes                                              */
/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void  UART_SendString            (char *string);

extern void  OMAP24xx_gpt_init          (void);
extern void  OMAP24xx_gpt_start         (void);
extern void  OMAP24xx_gpt_stop          (void);
extern float OMAP24xx_get_elapsed_time  (void);

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
    FSR_STACK_VAR;

    FSR_STACK_END;

    FSR_OAM_InitMemStat();

    /* if MMU is enabled, cache should be synchronized with DRAM
       when data in DRAM is transfered from/to OneNAND. */
    gbEnableMMU = FALSE32;
    if (ARM_IsMMUOn() == TRUE32)
    {
        gbEnableMMU = TRUE32;
    }

    gnCacheLineSize = ARM_GetDCacheLineSize();
#ifndef FSR_UBOOT
    OMAP24xx_gpt_init();
#endif /* FSR_UBOOT */
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

    gnMallocResetPtr = gnMallocPtr;
}

/**
 * @brief           This function resets malloc pointer
 *
 * @param[in]       bReset    : if TRUE32, Malloc pointer is set as 0
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

    if (bReset == TRUE32)
    {
        gnMallocPtr = 0;
    }
    else
    {
        gnMallocPtr = gnMallocResetPtr;
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
    UINT32  nAlignSize;
    FSR_STACK_VAR;

    FSR_STACK_END;
    
#if defined FSR_OAM_CHK_HEAPUSAGE
    gnFSRHeapUsage += nSize;
    gnFSRNumOfMemReqs++;
#endif

    nAlignSize = nSize / sizeof(UINT32);
    if (nSize % sizeof(UINT32))
        nAlignSize++;

    gnMallocPtr += nAlignSize;
    if (gnMallocPtr > LOCAL_MEM_SIZE)
        return NULL;

    return (VOID *) &(gaMemBuf[gnMallocPtr - nAlignSize]);
}

/**
 * @brief           This function allocates memory
 *
 * @param[in]       nSize    : Size to be allocated
 * @param[in]       nMemType : Memory type to be allocated
 * 
 * @return          Pointer of allocated memory
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID*
FSR_OAM_MallocExt(UINT32    nPDev,
                  UINT32    nSize,
                  UINT32    nMemType)
{

    UINT32  nAlignSize;
    UINT32  nVol;

    nVol = nPDev / FSR_MAX_VOLS;

    if (nMemType == FSR_OAM_LOCAL_MEM)
    {
        return FSR_OAM_Malloc(nSize);
    }
    else /* nMemType == FSR_OAM_SHARED_MEM */
    {
#if defined(FSR_USE_DUAL_CORE)
        nAlignSize = nSize / sizeof(UINT32);
        if (nSize % sizeof(UINT32))
        {
            nAlignSize++;
        }

        gnSMallocPtr[nVol] += nAlignSize;
        if (gnSMallocPtr[nVol] > gnShMemMaxSize[nVol])
        {
            return NULL;
        }

        return (VOID *) (gnShMemBaseAddress[nVol] + (gnSMallocPtr[nVol] - nAlignSize) * 4);
#else
        return NULL;
#endif
    }
}

/**
 * @brief           This function frees memory
 *
 * @param[in]      *pMem     : Pointer to be free
 * @param[in]       nMemType : Memory type to be free
 * 
 * @return         none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID
FSR_OAM_FreeExt(UINT32      nPDev,
                VOID       *pMem,
                UINT32      nMemType)
{
    UINT32  nVol;

    nVol = nPDev / FSR_MAX_VOLS;

    if (nMemType == FSR_OAM_LOCAL_MEM)
    {
        FSR_OAM_Free(pMem);
    }
    else /* nMemType == FSR_OAM_SHARED_MEM */
    {
#if defined(FSR_USE_DUAL_CORE)
        gnSMallocPtr[nVol] = SHARED_MEMORY_RESET;
#endif
    }
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

    /* NO semaphore APIs in OSLess environemts */

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

    /* NO semaphore APIs in OSLess environemts */

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

    /* NO semaphore APIs in OSLess environemts */

    return TRUE32;
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
    static UINT8   aStr[4096];
    va_list ap;
    int i = 0x44;
    va_start(ap, pFmt);
    i = 0x45;
    vsprintf((char *) aStr, (char *) pFmt, ap);
    i = 0x46;
    
    UART_SendString((char *) aStr);
    i = 0x47;
    
    va_end(ap);
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
    UINT32  nPhyIntId;
    FSR_STACK_VAR;

    FSR_STACK_END;

    nPhyIntId = FSR_PAM_GetPhyIntID(nLogIntId);

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
    UINT32  nPhyIntId;
    FSR_STACK_VAR;

    FSR_STACK_END;

    nPhyIntId = FSR_PAM_GetPhyIntID(nLogIntId);

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
#ifndef FSR_UBOOT
    OMAP24xx_gpt_start();
#endif /* FSR_UBOOT */
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
#ifndef FSR_UBOOT
    OMAP24xx_gpt_stop();
#endif /* FSR_UBOOT */
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
#ifndef FSR_UBOOT
    return (UINT32) (OMAP24xx_get_elapsed_time() * 1000000);
#else
	return 0;
#endif /* FSR_UBOOT */
}

/**
 * @brief           This function invalidates data cache
 *
 * @param[in]       nVirAddr : virtual address
 * @param[in]       nSize    : size to be invalidated
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PRIVATE VOID
_InvalidateDCache(UINT32     nVirAddr,
                  UINT32     nSize)
{
    UINT32      nCacheLineSize = gnCacheLineSize;
    FSR_STACK_VAR;

    FSR_STACK_END;

    if (gbEnableMMU == TRUE32)
    {
        if ((nVirAddr & (nCacheLineSize - 1)) != 0)
        {
            nSize += nCacheLineSize;
        }

        /* Cache Invalidate (flush) */
        while (nSize > 0)
        {
            ARM_InvalidateDCacheMVA(ARM_GET_MVA(nVirAddr, nCacheLineSize));
            nVirAddr    += nCacheLineSize;
            nSize       -= nCacheLineSize;
        }
    }
}

/**
 * @brief           This function flush data cache of head and tail of virtual address
 *
 * @param[in]       nVirAddr : virtual address
 * @param[in]       nSize    : size to be invalidated
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PRIVATE VOID
_FlushDCacheOfHeadNTail(UINT32      nVirAddr,
                        UINT32      nSize)
{
    UINT32      nCacheLineSize = gnCacheLineSize;
    FSR_STACK_VAR;

    FSR_STACK_END;

    if (gbEnableMMU == TRUE32)
    {
        /* Cache Clean */
        if ((nVirAddr & (nCacheLineSize - 1)) != 0)
        {
            ARM_CleanDCacheMVA(ARM_GET_MVA(nVirAddr, nCacheLineSize));           /* first location */
            ARM_CleanDCacheMVA(ARM_GET_MVA(nVirAddr + nSize, nCacheLineSize));   /*  last location */
        }
    }
}

/**
 * @brief           This function flushes data cache
 *
 * @param[in]       nVirAddr : virtual address
 * @param[in]       nSize    : size to be invalidated
 *
 * @return          none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PRIVATE VOID
_FlushDCache(UINT32      nVirAddr,
             UINT32      nSize)
{
    UINT32      nCacheLineSize = gnCacheLineSize;
    FSR_STACK_VAR;

    FSR_STACK_END;

    if (gbEnableMMU == TRUE32)
    {
        /* Flush cache */
        if ((nVirAddr & (nCacheLineSize - 1)) != 0)
        {
            nSize += nCacheLineSize;
        }

        while ((INT32)nSize > 0)
        {
            ARM_CleanDCacheMVA(ARM_GET_MVA(nVirAddr, nCacheLineSize));
            nVirAddr += nCacheLineSize;
            nSize    -= nCacheLineSize;
        }
    }
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

    /* dma global reg set-up */
    DMA4_CSDP0 = 0<<21 | 1<<20 | 0<<19 | 1<<18 | 2<<16 | 1<14 | 1<<13
                 | 0<<9 | 2<<7 | 1<<6 | 0<<2 | 1<<0;

    // MPU L1 interrupt masked
    DMA4_CICR0 = 0xFF;

    /* channel frame number */
    DMA4_CFN0 = 0x1;

    return FSR_OAM_SUCCESS;
}

/**
 * @brief           This function starts DMA operation
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
PRIVATE INT32
_StartDMA(UINT32     nVirDstAddr,
          UINT32     nVirSrcAddr,
          UINT32     nSize)
{
    UINT32      nPaSrc;
    UINT32      nPaDst;

    FSR_STACK_VAR;

    FSR_STACK_END;

    /* Destnation is OneNAND */
    nPaDst = ARM_Va2Pa(nVirDstAddr);

    /* Src is SDRAM   */
    nPaSrc = ARM_Va2Pa(nVirSrcAddr);

    DMA4_CSR0  = 0xff;
    DMA4_CSSA0 = (unsigned int) nPaSrc;
    DMA4_CDSA0 = (unsigned int) nPaDst;
    DMA4_CEN0  = nSize >> 1;     /* channel element number */

    DMA4_CCR0  = 1<<24 | 0<<23 | 0<<22 | 0<<21 | 0<<19 | 1<<18 | 0<<17 
                 | 0<<16 | 1<<14 | 1<<12 | 1<<8 | 1<<7 | 1<<6 | 1<<5 | 0;

    return FSR_OAM_SUCCESS;
}

/**
 * @brief           This function waits until DMA is finished
 *
 * @return          FSR_OAM_SUCCESS
 * @return          FSR_OAM_TIMEOUT
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PRIVATE INT32
_WaitDMAReady(VOID)
{
    FSR_STACK_VAR;

    FSR_STACK_END;

    /* poll if dma finished */
    while (!(DMA4_CSR0 & 0x8))
    {
        /*polling if DMA finished */
    }

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

    _FlushDCacheOfHeadNTail(nVirDstAddr, nSize);

    _StartDMA(nVirDstAddr, nVirSrcAddr, nSize);

    _InvalidateDCache(nVirDstAddr, nSize);

    _WaitDMAReady();

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

    _FlushDCache(nVirSrcAddr, nSize);

    ARM_DrainWBuf();

    _StartDMA(nVirDstAddr, nVirSrcAddr, nSize);

    _WaitDMAReady();

    return FSR_OAM_SUCCESS;
}
VOID
FSR_OAM_Memcpy(VOID *pDst, VOID *pSrc, UINT32 nLen)
{
#if defined(OAM_USE_STDLIB)
        memcpy(pDst, pSrc, nLen);
#else /* OAM_USE_STDLIB */
		memcpy32(pDst, pSrc, nLen);
#endif /* OAM_USE_STDLIB */
}

VOID
FSR_OAM_Memset(VOID *pDst, UINT8 nV, UINT32 nLen)
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

INT32
FSR_OAM_Memcmp(VOID *pSrc, VOID *pDst, UINT32 nLen)
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

#ifndef FSR_UBOOT
static char * number(char * str, long num, int base, int size, int precision
        ,int type)
{
        char c,sign,tmp[66];
        const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
        int i;

        if (type & LARGE)
                digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if (type & LEFT)
                type &= ~ZEROPAD;
        if (base < 2 || base > 36)
                return 0;
        c = (type & ZEROPAD) ? '0' : ' ';
        sign = 0;
        if (type & SIGN) {
                if (num < 0) {
                        sign = '-';
                        num = -num;
                        size--;
                } else if (type & PLUS) {
                        sign = '+';
                        size--;
                } else if (type & SPACE) {
                        sign = ' ';
                        size--;
                }
        }
        if (type & SPECIAL) {
                if (base == 16)
                        size -= 2;
                else if (base == 8)
                        size--;
        }
        i = 0;
        if (num == 0)
                tmp[i++]='0';
        else while (num != 0)
                tmp[i++] = digits[do_div(num,base)];
        if (i > precision)
                precision = i;
        size -= precision;
        if (!(type&(ZEROPAD+LEFT)))
                while(size-->0)
                        *str++ = ' ';
        if (sign)
                *str++ = sign;
        if (type & SPECIAL) {
                if (base==8)
                        *str++ = '0';
                else if (base==16) {
                        *str++ = '0';
                        *str++ = digits[33];
                }
        }
        if (!(type & LEFT))
                while (size-- > 0)
                        *str++ = c;
        while (i < precision--)
                *str++ = '0';
        while (i-- > 0)
                *str++ = tmp[i];
        while (size-- > 0)
      *str++ = ' ';
        return str;
}

static int skip_atoi(const char **s)
{
        int i=0;

        while (is_digit(**s))
                i = i*10 + *((*s)++) - '0';
        return i;
}

size_t strnlen(const char * s, size_t count)
{
        const char *sc;

        for (sc = s; count-- && *sc != '\0'; ++sc)
                /* nothing */;
        return sc - s;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
        int len;
        unsigned long num;
        int i, base;
        char * str;
        const char *s;

        int flags;              /* flags to number() */

        int field_width;        /* width of output field */
        int precision;          /* min. # of digits for integers; max
                                   number of chars for from string */
        int qualifier;          /* 'h', 'l', or 'L' for integer fields */

        for (str=buf ; *fmt ; ++fmt) {
                if (*fmt != '%') {
                        *str++ = *fmt;
                        continue;
                }

                /* process flags */
                flags = 0;
                repeat:
                        ++fmt;          /* this also skips first '%' */
                        switch (*fmt) {
                                case '-': flags |= LEFT; goto repeat;
                                case '+': flags |= PLUS; goto repeat;
                                case ' ': flags |= SPACE; goto repeat;
                                case '#': flags |= SPECIAL; goto repeat;
                                case '0': flags |= ZEROPAD; goto repeat;
                                }

                /* get field width */
                field_width = -1;
                if (is_digit(*fmt))
                        field_width = skip_atoi(&fmt);
                else if (*fmt == '*') {
                        ++fmt;
                        /* it's the next argument */
                        field_width = va_arg(args, int);
                        if (field_width < 0) {
                                field_width = -field_width;
                                flags |= LEFT;
                        }
                }

                /* get the precision */
                precision = -1;
                if (*fmt == '.') {
                        ++fmt;
                        if (is_digit(*fmt))
                                precision = skip_atoi(&fmt);
                        else if (*fmt == '*') {
                                ++fmt;
                                /* it's the next argument */
                                precision = va_arg(args, int);
                        }
                        if (precision < 0)
                                precision = 0;
                }

                /* get the conversion qualifier */
                qualifier = -1;
                if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
                        qualifier = *fmt;
                        ++fmt;
                }

                /* default base */
                base = 10;

                switch (*fmt) {
                case 'c':
                        if (!(flags & LEFT))
                                while (--field_width > 0)
                                        *str++ = ' ';
                        *str++ = (unsigned char) va_arg(args, int);
                        while (--field_width > 0)
                                *str++ = ' ';
                        continue;

                case 's':
                        s = va_arg(args, char *);
                        if (!s)
                                s = "<NULL>";

                        len = strnlen(s, precision);
                        if (!(flags & LEFT))
                                while (len < field_width--)
                                        *str++ = ' ';
                        for (i = 0; i < len; ++i)
                                *str++ = *s++;
                        while (len < field_width--)
                                *str++ = ' ';
                        continue;

                case 'p':
                        if (field_width == -1) {
                                field_width = 2*sizeof(void *);
                                flags |= ZEROPAD;
                        }
                        str = number(str,
                                (unsigned long) va_arg(args, void *), 16,
                                field_width, precision, flags);
                        continue;


                case 'n':
                        if (qualifier == 'l') {
                                long * ip = va_arg(args, long *);
                                *ip = (str - buf);
                        } else {
                                int * ip = va_arg(args, int *);
                                *ip = (str - buf);
                        }
                        continue;

                case '%':
                        *str++ = '%';
                        continue;

                /* integer number formats - set up the flags and "break" */
                case 'o':
                        base = 8;
                        break;

                case 'X':
                        flags |= LARGE;
                case 'x':
                        base = 16;
                        break;

                case 'd':
                case 'i':
                        flags |= SIGN;
                case 'u':
                        break;

                default:
                        *str++ = '%';
                        if (*fmt)
                                *str++ = *fmt;
                        else
                                --fmt;
                        continue;
                }
                if (qualifier == 'l')
                        num = va_arg(args, unsigned long);
                else if (qualifier == 'h') {
                        num = (unsigned short) va_arg(args, int);
                        if (flags & SIGN)
                                num = (short) num;
                } else if (flags & SIGN)
                        num = va_arg(args, int);
                else
                        num = va_arg(args, unsigned int);
                str = number(str, num, base, field_width, precision, flags);
        }
        *str = '\0';
        return str-buf;
}
#endif /* FSR_UBOOT */

#if !defined(OAM_USE_STDLIB) && !defined(FSR_UBOOT)
void memcpy(VOID *pDst, VOID *pSrc, UINT32 nLen)
{
	FSR_OAM_Memcpy(pDst, pSrc, nLen);
}
#endif /* OAM_USE_STDLIB */

VOID raise(VOID) {}

