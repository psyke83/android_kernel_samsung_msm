/**
 *   @mainpage   Opened Low Level device Driver
 *
 *   @section Intro
 *       Low Level device Driver for OneNAND
 *    
 *    @section  Copyright
 *            COPYRIGHT. 2003-2009 SAMSUNG ELECTRONICS CO., LTD.               
 *                      ALL RIGHTS RESERVED
 *
 *   Permission is hereby granted to licensees of Samsung Electronics
 *   Co., Ltd. products to use or abstract this computer program for the
 *   sole purpose of implementing a product based on Samsung
 *   Electronics Co., Ltd. products. No other rights to reproduce, use,
 *   or disseminate this computer program, whether in part or in whole,
 *   are granted.
 *
 *   Samsung Electronics Co., Ltd. makes no representation or warranties
 *   with respect to the performance of this computer program, and
 *   specifically disclaims any responsibility for any damages,
 *   special or consequential, connected with the use of this program.
 *
 *     @section Description
 *
 */

/**
 * @file      Open_PAM_Win32.c
 * @brief     This file contain the Platform Adaptation Modules for Win32 OS
 * @author    JeongWook Moon
 * @date      14-DEC-2007
 * @remark
 * REVISION HISTORY
 * @n  14-DEC-2007 [JeongWook Moon] : first writing
 *
 */

#include "O1N_LLD_1N.h"

/*****************************************************************************/
/* Global variables definitions                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Local #defines                                                            */
/*****************************************************************************/
#define ONENAND_PHY_BASE_ADDR       (0x00100000)

/*****************************************************************************/
/* Local typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local constant definitions                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/

PRIVATE UINT32           gnBaseAddr;

/**
 * @brief          This function initializes PAM
 *                 
 *
 * @param[out]     nBaseAddr    : Base address of mapped DataRAM

 * @return         OPEN_PAM_SUCCESS
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 *
 */
PUBLIC INT32
O1N_PAM_Init(UINT32 *pBaseAddr)
{
#if defined (FSR_ONENAND_EMULATOR)
    gnBaseAddr    = 0x20000000;
#else
    gnBaseAddr    = ONENAND_PHY_BASE_ADDR;
#endif

    *pBaseAddr = gnBaseAddr;

    return O1N_PAM_SUCCESS;
}



/**
 * @brief           This function transfers data to NAND
 *
 * @param[in]      *pDst  : Destination array Pointer to be copied
 * @param[in]      *pSrc  : Source data allocated Pointer
 * @param[in]      *nSize : length to be transferred
 *
 * @return          none
 *
 * @author          JeongWook Moon
 * @version         2.0.0
 *
 */
PUBLIC VOID
O1N_PAM_TransToNAND(volatile VOID *pDst,
                    VOID *pSrc,
                    UINT32 nSize)
{
    O1N_OAM_MEMCPY((VOID *) pDst, pSrc, nSize);
}

/**
 * @brief           This function transfers data from NAND
 *
 * @param[in]      *pDst  : Destination array Pointer to be copied
 * @param[in]      *pSrc  : Source data allocated Pointer
 * @param[in]      *nSize : length to be transferred
 *
 * @return          none
 *
 * @author          JeongWook Moon
 * @version         2.0.0
 *
 */
PUBLIC VOID
O1N_PAM_TransFromNAND(VOID *pDst,
                      volatile VOID *pSrc,
                      UINT32 nSize)
{
    O1N_OAM_MEMCPY(pDst, (VOID *) pSrc, nSize);
}


