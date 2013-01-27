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
 * @file      Open_PAM.h
 * @brief     This file contains the definition and protypes of exported
 *              functions for Platform Adaptation Module of OPEN.
 * @author    JeongWook Moon
 * @date      14-DEC-2007
 * @remark
 * REVISION HISTORY
 * @n  14-DEC-2007 [JeongWook Moon] : first writing
 *
 */

#ifndef _OPEN_PAM_H_
#define _OPEN_PAM_H_

/*****************************************************************************/
/* Definitions of PAM                                                        */
/*****************************************************************************/
#define     O1N_PAM_NOT_MAPPED          0xFFFFFFFF /** Device not mapped 
                                                      in platform memory map */

/*****************************************************************************/
/* Major Return value of OPEN_PAM_XXX()                                       */
/*****************************************************************************/
/* Common value for OPEN_PAM_XXX()*/
#define     O1N_PAM_SUCCESS            O1N_RETURN_VALUE(0, 0x3, 0x0000, 0x0000)
#define     O1N_PAM_CRITICAL_ERROR     O1N_RETURN_VALUE(1, 0x3, 0x0001, 0x0000)
#define     O1N_PAM_NOT_INITIALIZED    O1N_RETURN_VALUE(1, 0x3, 0x0002, 0x0000)


/*****************************************************************************/
/* Exported Function Prototype of PAM                                        */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

INT32   O1N_PAM_Init(UINT32  *pBaseAddr);

/*****************************************************************************/
/* APIs to optimize NAND transfer performance                                */
/*****************************************************************************/
VOID    O1N_PAM_TransToNAND(volatile VOID  *pDst, 
                                     VOID  *pSrc, 
                                     UINT32 nSize);
VOID    O1N_PAM_TransFromNAND(         VOID  *pDst, 
                              volatile VOID  *pSrc, 
                                       UINT32 nSize);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* _OPEN_PAM_H_ */
