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
 * @file      Open_LLD_1N.h
 * @brief     This is Low level driver of OneNAND
 * @author    JeongWook Moon
 * @date      11-DEC-2007
 * @remark
 * REVISION HISTORY
 * @n  11-DEC-2007 [JeongWook Moon] : first writing
 *
 */

#ifndef _O1N_LLD_1N_H_
#define _O1N_LLD_1N_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*****************************************************************************/
/* Basic type defines                                                        */
/*****************************************************************************/
typedef     unsigned int                        UINT32;
typedef     int                                 INT32;
typedef     unsigned short                      UINT16;
typedef     short                               INT16;
typedef     unsigned char                       UINT8;
typedef     signed char                         INT8;


#ifndef     VOID
#ifdef      _WIN32
typedef     void                                VOID;
#else
#define     VOID                                void
#endif
#endif

typedef     UINT32                              BOOL32;
typedef     UINT32                              SM32;


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
/* Basic Constants                                                           */
/*****************************************************************************/
#ifndef     FALSE32
#define     FALSE32                             (BOOL32) 0
#endif  

#ifndef     TRUE32
#define     TRUE32                              (BOOL32) 1
#endif

#ifndef     PUBLIC
#define     PUBLIC
#endif

#ifndef     PRIVATE
#define     PRIVATE                             static
#endif

#ifdef      TEXT
#undef      TEXT
#endif
#define     TEXT(x)                             (VOID *) (x)

#if defined (FSR_ONENAND_EMULATOR)
#define     O1N_LLD_PRINT(x)                    FSR_RTL_PRINT(x)
#else
VOID        O1N_LLD_DbgMsg(VOID *pStr, ...);
#define     O1N_LLD_PRINT(x)                    O1N_OAM_DbgMsg x

#endif

/*****************************************************************************/
/* Return value MACRO definition                                             */
/*                                                                           */
/* bit 31      /  1 bit  / err / if return value is error, 1. otherwise, 0   */
/* bit 30 ~ 28 /  3 bits / lay / STL:0, BML:1, LLD:2, PAM:3, OAM:4           */
/* bit 27 ~ 16 / 12 bits / major error value (0x000 ~ 0xFFF)                 */
/* bit 16 ~ 00 / 16 bits / minor error value (0x0000 ~ 0xFFFF)               */
/*****************************************************************************/
#define     O1N_RETURN_MAJOR(err)          (INT32)((err) & 0xFFFF0000)
#define     O1N_RETURN_MINOR(err)          (INT32)((err) & 0x0000FFFF)
#define     O1N_RETURN_VALUE(err, lay, maj, min)                            \
                                            (INT32)(((UINT32)((err) & 0x00000001) << 31) | \
                                            ((UINT32)((lay) & 0x00000007) << 28) | \
                                            ((UINT32)((maj) & 0x00000FFF) << 16) | \
                                            (UINT32)((min) & 0x0000FFFF))

/*****************************************************************************/
/* MACROs                                                                    */
/*****************************************************************************/

/* nNANDType of FSRDevSpec can have values listed below                      */
#define     O1N_LLD_FLEX_ONENAND                (0x0000)
#define     O1N_LLD_SLC_ONENAND                 (0x0001)

#define     O1N_LLD_MAX_DIES                    (2)

#define     O1N_LLD_SECTOR_SIZE                 (512)
#define     O1N_LLD_SPARE_SIZE                  (16)

/*****************************************************************************/
/* nFlag Values                                                              */
/*****************************************************************************/
#define     O1N_LLD_FLAG_1X_PROGRAM             (0x00000000)
#define     O1N_LLD_FLAG_2X_PROGRAM             (0x00000002)

#define     O1N_LLD_FLAG_1X_CHK_BADBLOCK        (0x00000001)
#define     O1N_LLD_FLAG_2X_CHK_BADBLOCK        (0x00000002)

#define     O1N_LLD_FLAG_USE_DATARAM0           (0x00000000)
#define     O1N_LLD_FLAG_USE_DATARAM1           (0x00000010)

#define     O1N_LLD_FLAG_LOCK_OTP_BLOCK         (0xFC)
#define     O1N_LLD_FLAG_LOCK_1ST_BLOCK_OTP     (0xF3)
#define     O1N_LLD_FLAG_LOCK_BOTH_OTP          (0xF0)

/* 3rd parameter of O1N_LLD_ChangeLockStat() */
#define     O1N_LLD_FLAG_UNLOCK_BLOCK           (0x00000001)
#define     O1N_LLD_FLAG_UNLOCK_ALL_BLOCK       (0x00000002)
#define     O1N_LLD_FLAG_LOCK_BLOCK             (0x00000003)
#define     O1N_LLD_FLAG_LOCK_TIGHT_BLOCK       (0x00000004)

#define     O1N_LLD_FLAG_OTP_BLK_STATUS         (0x00000001)
#define     O1N_LLD_FLAG_NORMAL_BLK_STATUS      (0x00000002)

#define     O1N_LLD_FLAG_HOT_RESET              (0x00000001)
#define     O1N_LLD_FLAG_NCORE_RESET            (0x00000002)

/*****************************************************************************/
/* LLD function Return Values                                                */
/*****************************************************************************/
#define     O1N_LLD_RETURN_SUCCESS              (0x20000000)
#define     O1N_LLD_RETURN_ERROR                (0x10000000)
#define     O1N_LLD_INIT_GOODBLOCK              (0x00000001)
#define     O1N_LLD_INIT_BADBLOCK               (0X00000002)
#define     O1N_LLD_RETURN_1STPLN_ERROR         (0x00000100)
#define     O1N_LLD_RETURN_2NDPLN_ERROR         (0x00000200)
#define     O1N_LLD_RETURN_READ_DISTURBANCE     (0x00000300)

/* return value of FSR_LLD_OTP_GET_INFO                                */
#define     O1N_LLD_OTP_1ST_BLK_LOCKED          (0x00000001)
#define     O1N_LLD_OTP_1ST_BLK_UNLKED          (0x00000002)
#define     O1N_LLD_OTP_OTP_BLK_LOCKED          (0x00000010)
#define     O1N_LLD_OTP_OTP_BLK_UNLKED          (0x00000020)

#define     O1N_LLD_BLK_UNLKED                  (0x00000000)
#define     O1N_LLD_BLK_LOCKED                  (0x00000001)

#define     O1N_LLD_MAX_DIES                    (2)


/** external memory address MAP                                              */
typedef struct
{
    volatile UINT8    nBootM0[512];
    volatile UINT8    nBootM1[512];
    volatile UINT8    nDataMB00[512];
    volatile UINT8    nDataMB01[512];
    volatile UINT8    nDataMB02[512];
    volatile UINT8    nDataMB03[512];
    volatile UINT8    nDataMB10[512];
    volatile UINT8    nDataMB11[512];
    volatile UINT8    nDataMB12[512];
    volatile UINT8    nDataMB13[512];

    volatile UINT8    nRsv1[59 * 1024];

    volatile UINT8    nBootS0[16];
    volatile UINT8    nBootS1[16];
    volatile UINT8    nDataSB00[16];
    volatile UINT8    nDataSB01[16];
    volatile UINT8    nDataSB02[16];
    volatile UINT8    nDataSB03[16];
    volatile UINT8    nDataSB10[16];
    volatile UINT8    nDataSB11[16];
    volatile UINT8    nDataSB12[16];
    volatile UINT8    nDataSB13[16];

    volatile UINT8    nRsv2[8032];
    volatile UINT8    nRsv3[24 * 1024];
    volatile UINT8    nRsv4[8 * 1024];
    volatile UINT8    nRsv5[16 * 1024];

    volatile UINT16   nMID;             /* offset : 0x1E000 */
    volatile UINT16   nDID;             /* offset : 0x1E002 */
    volatile UINT16   nVerID;           /* offset : 0x1E004 */
    volatile UINT16   nDataBufSize;     /* offset : 0x1E006 */
    volatile UINT16   nBootBufSize;     /* offset : 0x1E008 */
    volatile UINT16   nBufAmount;       /* offset : 0x1E00a */
    volatile UINT16   nTech;            /* offset : 0x1E00c */

    volatile UINT8    nRsv6[0x1F2];

    volatile UINT16   nStartAddr1;      /* offset : 0x1E200 */
    volatile UINT16   nStartAddr2;      /* offset : 0x1E202 */
    volatile UINT16   nStartAddr3;      /* offset : 0x1E204 */
    volatile UINT16   nStartAddr4;      /* offset : 0x1E206 */
    volatile UINT16   nStartAddr5;      /* offset : 0x1E208 */
    volatile UINT16   nStartAddr6;      /* offset : 0x1E20a */
    volatile UINT16   nStartAddr7;      /* offset : 0x1E20c */
    volatile UINT16   nStartAddr8;      /* offset : 0x1E20e */

    volatile UINT8    nRsv7[0x1F0];

    volatile UINT16   nStartBuf;        /* offset : 0x1E400 */

    volatile UINT8    nRsv8[0xE];
    volatile UINT8    nRsv9[0x30];

    volatile UINT16   nCmd;             /* offset : 0x1E440 */
    volatile UINT16   nSysConf1;        /* offset : 0x1E442 */

    volatile UINT8    nRsv10[0x1C];
    volatile UINT8    nRsv11[0x20];

    volatile UINT16   nCtrlStat;        /* offset : 0x1E480 */
    volatile UINT16   nInt;             /* offset : 0x1E482 */

    volatile UINT8    nRsv12[0x14];

    volatile UINT16   nStartBlkAddr;    /* offset : 0x1E498 */
    volatile UINT16   nRsv13;           /* offset : 0x1E49A */
    volatile UINT16   nWrProtectStat;   /* offset : 0x1E49C */

    volatile UINT8    nRsv14[0x1962];

    volatile UINT16   nEccStat;         /* offset : 0x1FE00 */
    volatile UINT16   nEccResult[8];    /* offset : 0x1FE02 ~ 0x1FE10 */

    volatile UINT8    nRsv15[0x1EE];
} OneNANDReg;

/**
 * @brief  data structure for copy-back random-in arguement
 */
typedef struct
{
    UINT16    nOffset;          /**< nOffset should be even number
                                     main area range  :          0 ~ 2046
                                     spare area range :       2048 ~ 2110       */
    UINT16    nNumOfBytes;      /**< Random In Bytes                         */
    VOID     *pBuf;             /**< Data Buffer Pointer                     */
} Open1NRndInArg;

/**
 * @brief  data structure for copy-back arguement
 */
typedef struct
{
    UINT16          nSrcPbn;       /**< Copy Back Source Vun                    */
    UINT16          nSrcPgOffset;
    UINT16          nDstPbn;       /**< Copy Back Dest.  Vun                    */
    UINT16          nDstPgOffset;
    UINT32          nRndInCnt;     /**< Random In Count                         */
    Open1NRndInArg *pstRndInArg;   /**< RndInArg Array pointer                  */
} Open1NCpBkArg;

/**
 * @brief data structure which can be obtained by FSR_LLD_GetDevSpec function call
 */
typedef struct
{
            UINT16      nNumOfBlks;     /**< # of blocks in NAND device      */
            UINT16      nNumOfPlanes;   /**< # of planes in die              */
            UINT16      nBlksForSLCArea[O1N_LLD_MAX_DIES];/**< # of blks for SLC
                                                             area            */


            UINT16      nSparePerSct;   /**< # of bytes of spare of a sector */
            UINT16      nSctsPerPG;     /**< # of sectors per page           */
            UINT16      nNumOfBlksIn1stDie; /**< # of blocks in 1st Die      */
            UINT16      nDID;           /**< Device ID                       */

            UINT32      nPgsPerBlkForSLC;/**< # of pages per block in SLC area*/
            UINT32      nPgsPerBlkForMLC;/**< # of pages per block in MLC area*/

            UINT16      nNumOfDies;     /**< # of dies in NAND device        */
            UINT16      nRsvBlksInDev;  /**< # of total bad blocks (init + run)*/
            UINT16      nNANDType;      /**< NAND types
                                           - FSR_LLD_FLEX_ONENAND
                                           - FSR_LLD_SLC_NAND
                                           - FSR_LLD_MLC_NAND
                                           - FSR_LLD_SLC_ONENAND             */
} Open1NSpec;

/*****************************************************************************/
/* OneNAND System Configureation1 Register Values                            */
/*****************************************************************************/
#define     O1N_LLD_CONF1_SYNC_READ             (UINT16) (0x8000)
#define     O1N_LLD_CONF1_ASYNC_READ            (UINT16) (0x0000)

#define     O1N_LLD_CONF1_BST_RD_LATENCY_3      (UINT16) (0x3000)     /*   min   */
#define     O1N_LLD_CONF1_BST_RD_LATENCY_4      (UINT16) (0x4000)     /* default */
#define     O1N_LLD_CONF1_BST_RD_LATENCY_5      (UINT16) (0x5000)
#define     O1N_LLD_CONF1_BST_RD_LATENCY_6      (UINT16) (0x6000)
#define     O1N_LLD_CONF1_BST_RD_LATENCY_7      (UINT16) (0x7000)

#define     O1N_LLD_CONF1_BST_LENGTH_CONT       (UINT16) (0x0000)     /* default */
#define     O1N_LLD_CONF1_BST_LENGTH_4WD        (UINT16) (0x0200)
#define     O1N_LLD_CONF1_BST_LENGTH_8WD        (UINT16) (0x0400)
#define     O1N_LLD_CONF1_BST_LENGTH_16WD       (UINT16) (0x0600)
#define     O1N_LLD_CONF1_BST_LENGTH_32WD       (UINT16) (0x0800)     /* N/A on spare */
#define     O1N_LLD_CONF1_BST_LENGTH_1KWD       (UINT16) (0x0A00)     /* N/A on spare, sync. burst block read only */

#define     O1N_LLD_CONF1_ECC_ON                (UINT16) (0xFEFF)
#define     O1N_LLD_CONF1_ECC_OFF               (UINT16) (0x0100)     /* (~CONF1_ECC_ON) */

#define     O1N_LLD_CONF1_RDY_POLAR             (UINT16) (0x0080)
#define     O1N_LLD_CONF1_INT_POLAR             (UINT16) (0x0040)
#define     O1N_LLD_CONF1_IOBE_ENABLE           (UINT16) (0x0020)

#define     O1N_LLD_CONF1_BWPS_UNLOCKED         (UINT16) (0x0001)

#define     O1N_LLD_CONF1_HF_ON                 (UINT16) (0x0004)
#define     O1N_LLD_CONF1_HF_OFF                (UINT16) (0xFFFB)
#define     O1N_LLD_CONF1_RDY_CONF              (UINT16) (0x0010)

/*****************************************************************************/
/* exported common APIs                                                      */
/*****************************************************************************/
PUBLIC INT32   O1N_LLD_Open(UINT32             nBaseAddr);
PUBLIC INT32   O1N_LLD_Close(VOID);
PUBLIC INT32   O1N_LLD_Write(UINT32            nPbn,
                             UINT32            nPgOffset,
                             UINT8            *pMBuf,
                             UINT8            *pSBuf,
                             UINT32            nFlag);
PUBLIC INT32   O1N_LLD_WriteOTP(UINT32         nPbn,
                                UINT32         nPgOffset,
                                UINT8         *pMBuf,
                                UINT8         *pSBuf);
PUBLIC INT32   O1N_LLD_Read(UINT32             nPbn,
                            UINT32             nPgOffset,
                            UINT8             *pMBuf,
                            UINT8             *pSBuf,
                            UINT32             nFlag);
PUBLIC INT32   O1N_LLD_ReadOTP(UINT32          nPbn,
                               UINT32          nPgOffset,
                               UINT8          *pMBuf,
                               UINT8          *pSBuf);
PUBLIC INT32   O1N_LLD_Erase(UINT32            nPbn);
PUBLIC INT32   O1N_LLD_ChkBadBlk(UINT32        nPbn,
                                 UINT32        nFlag);
PUBLIC INT32   O1N_LLD_GetDevSpec(Open1NSpec  *pstDevSpec);
PUBLIC INT32   O1N_LLD_ChangeLockStat(UINT32   nPbn,
                                      UINT32   nType);
PUBLIC INT32   O1N_LLD_ProtectOTP(UINT32       nLockFlag);
PUBLIC INT32   O1N_LLD_CopyBack(Open1NCpBkArg *pstCpArg,
                                UINT32         nFlag);
PUBLIC INT32   O1N_LLD_GetBlockStat(UINT32     nPbn,
                                    UINT32    *pnLockStat,
                                    UINT32     nFlag);
PUBLIC INT32   O1N_LLD_Reset(UINT32            nFlag);
#ifdef __cplusplus
}
#endif /* __cplusplus */

/*****************************************************************************/
/* Include exported header files                                             */
/*****************************************************************************/
#include "O1N_OAM.h"
#include "O1N_PAM.h"

#endif /* _FSR_ONENAND_LLD_H_ */
