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
 * @file      Open_OAM.h
 * @brief     This file contains the definition and protypes of exported
 *              functions for OS Adaptation Module.
 * @author    JeongWook Moon
 * @date      14-DEC-2007
 * @remark
 * REVISION HISTORY
 * @n  14-DEC-2007 [JeongWook Moon] : first writing
 *
 */

#ifndef _O1N_OAM_H_
#define _O1N_OAM_H_


#define O1N_RTL_PRINT(x)        OPEN_OAM_DbgMsg x

#define O1N_DBG_PRINT(x)

#ifdef   TEXT
#undef   TEXT
#endif
#define TEXT(x)                 (VOID *) (x)

#include <string.h> /* for ARM ADS1.2 */
#define     O1N_OAM_MEMCPY(a, b, c)                 memcpy((a), (b), (c))
#define     O1N_OAM_MEMSET(a, b, c)                 memset((a), (b), (c))
#define     O1N_OAM_MEMCMP(a, b, c)                 memcmp((a), (b), (c))


/*****************************************************************************/
/* Major Return value of OPEN_OAM_XXX()                                       */
/*****************************************************************************/
/* Common value for OPEN_OAM_XXX() */
#define     O1N_OAM_SUCCESS             O1N_RETURN_VALUE(0, 0x4, 0x0000, 0x0000)
#define     O1N_OAM_NOT_INITIALIZED     O1N_RETURN_VALUE(1, 0x4, 0x0002, 0x0000)

/*****************************************************************************/
/* NULL #defines                                                             */
/*   If Each OAM header file defines NULL, following define is not used.     */
/*****************************************************************************/
#ifndef     NULL
#ifdef      __cplusplus
#define     NULL                0
#else
#define     NULL                ((void *)0)
#endif
#endif

/*****************************************************************************/
/* __FUNCTION__ / __FILE__ macro                                             */
/*****************************************************************************/
#ifndef __FUNCTION__
    #if (__CC_ARM == 1)
        #define __FUNCTION__ __func__   /* for ARM ADS1.2 */
    #elif defined(__linux__)
        /* Linux supports __FUNCTION__ */
    #else
        #define __FUNCTION__ "[__FUNCTION__]"
    #endif
#endif

#define __O1N_FUNC__        TEXT(__FUNCTION__)
#define __O1N_FILE__        TEXT(__FILE__)

/****************************************************************************/
/* assertion macro                                                          */
/****************************************************************************/
#if defined (O1N_ASSERT)
    #undef  O1N_ASSERT
    #define O1N_ASSERT(f)  {if ((f) == 0) \
                            {O1N_RTL_PRINT((TEXT("\n OPEN/Assertion Failed : %s,line:%d,func:%s\n"), \ 
                            (const unsigned char *) __O1N_FILE__, __LINE__, (const unsigned char *) __O1N_FUNC__));\
                            O1N_RTL_PRINT((TEXT("\n<log P1=\"100\" P2=\"POR\" P3=\"1\" P4=\"mammoth auto power off\" />\n")));\
                            while (1);}}
#else
    #define O1N_ASSERT(f)
#endif


/*****************************************************************************/
/* Miscellaneous Functions Wrappers                                          */
/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*****************************************************************************/
/* exported function prototype of OAM                                        */
/*****************************************************************************/
INT32    O1N_OAM_Init                   (VOID);

/*****************************************************************************/
/* extra APIs                                                                */
/*****************************************************************************/
VOID     O1N_OAM_DbgMsg                 (VOID *pStr, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* _OPEN_OAM_H_ */
