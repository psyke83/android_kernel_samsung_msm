/**
 *   @mainpage   Opened Low Level device Driver
 *
 *   @section Intro
 *       Low Level device Driver for OneNAND
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
 * @file      Open_OAM_OSLess.c
 * @brief     This file contain the OS Adaptation Modules for NO OS
 * @author    JeongWook Moon
 * @date      14-DEC-2007
 * @remark
 * REVISION HISTORY
 * @n  14-DEC-2007 [JeongWook Moon] : first writing
 *
 */

/* standard I/O include files */
#include    <stdio.h>
#include    <stdarg.h>
#include    <string.h>

/* OPEN include file */
#include    "O1N_LLD_1N.h"

/*****************************************************************************/
/* Global variables definitions                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Local #defines                                                            */
/*****************************************************************************/
#define     OPEN_OAM_CHK_HEAPUSAGE

#if defined (OPEN_OAM_ALLOC_SMALL_HEAP)
    #define     LOCAL_MEM_SIZE      ((32 * 1024) / sizeof(UINT32))
#elif defined (OPEN_OAM_ALLOC_MEDIUM_HEAP)
    #define     LOCAL_MEM_SIZE      ((5 * 100 * 1024) / sizeof(UINT32))
#else
    #define     LOCAL_MEM_SIZE      ((5 * 1024 * 1024) / sizeof(UINT32))
#endif

#define     ARM_GET_MVA (MVA, ClineSize)      (MVA & ~(ClineSize - 1))

/*****************************************************************************/
/* Local typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local constant definitions                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/

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

extern void  UART_SendString            (char *string);

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
 * @author          JeongWook Moon
 * @version         2.0.0
 *
 * @remark          OPEN_PAM_Init() should be called before OPEN_OAM_Init() is called.
 *                  
 */
PUBLIC INT32
O1N_OAM_Init(VOID)
{
    return O1N_OAM_SUCCESS;
}


/**
 * @brief           This function prints debug message
 *
 * @param[in]       *pFmt : NULL-terminated string to be printed
 *
 * @return          none
 *
 * @author          JeongWook Moon
 * @version         2.0.0
 * @remark          This function is used when OS can not support debug print API
 *
 */
static UINT8   gaStr[1024 * 8];
 
PUBLIC VOID
O1N_OAM_DbgMsg(VOID  *pFmt, ...)
{
    va_list ap;
/*
    DO NOT use     OPEN_STACK_VAR/OPEN_STACK_END macro
*/

    va_start(ap, pFmt);
    vsnprintf((char *) gaStr, (size_t) sizeof(gaStr), (char *) pFmt, ap);
    
    UART_SendString((char *) gaStr);
    
    va_end(ap);
}

