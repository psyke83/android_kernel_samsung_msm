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
 * @file      Open_LLD_1N.c
 * @brief     This is Low level driver of OneNAND
 * @author    Jeong-Wook Moon
 * @date      03-DEC-2007
 * @remark
 * REVISION HISTORY
 * @n  03-DEC-2007 [JeongWook Moon] : first writing
 *
 */


/*****************************************************************************/
/* Header file inclusions                                                    */
/*****************************************************************************/
#define     FSR_NO_INCLUDE_BML_HEADER
#define     FSR_NO_INCLUDE_STL_HEADER


#include    <string.h>
#if defined (FSR_ONENAND_EMULATOR)
#include    "FSR.h"
#include    "FSR_FOE_Interface.h"
#else
#include    <stdarg.h>
#endif /* #if defined(FSR_ONENAND_EMULATOR) */
#include    "O1N_LLD_1N.h"

/********************************************************************************/
/*   Local Configurations                                                       */
/*                                                                              */
/* - O1N_LLD_BIG_ENDIAN           : to support big endianess                    */
/* - FSR_ONENAND_EMULATOR         : to use OneNAND emulator                     */
/*                                  instead of real device (for debugging only) */
/********************************************************************************/

#undef      O1N_LLD_BIG_ENDIAN

/*****************************************************************************/
/* Local #defines                                                            */
/*****************************************************************************/

#define     O1N_LLD_MAX_DEVS                FSR_MAX_DEVS

/* OneNAND register : bit information */
#define     O1N_LLD_INT_READY               (0x8000)        /* interrupt status register   */
#define     O1N_LLD_INT_CLEAR               (0x0000)
#define     O1N_LLD_DBS_BASEBIT             (15)            /* start address 2 register    */
#define     O1N_LLD_DFS_BASEBIT             (15)            /* start address 1 register    */
#define     O1N_LLD_STATUS_ERROR            (0x0400)        /* controller status register  */
#define     O1N_LLD_ECC_NOERROR             (0x0000)        /* ECC status register 1, 2, 3, 4 */

#define     O1N_LLD_DDP_CHIP                (0x0008)        /* Device ID register          */

/* register masking values */
#define     O1N_LLD_MASK_DFS                (0x8000)
#define     O1N_LLD_MASK_DBS                (0x8000)
#define     O1N_LLD_MASK_OTPBL              (0x0020)
#define     O1N_LLD_MASK_OTPL               (0x0040)
#define     O1N_LLD_MASK_OTP                (O1N_LLD_MASK_OTPBL | O1N_LLD_MASK_OTPL)

#define     O1N_LLD_MASK_LOCKBIT            (0x4000)

#define     O1N_LLD_VALID_BLK_MARK          (0xFFFF)

/* register shift or mask value */
#define     O1N_LLD_FPA_SHIFT               (2)
#define     O1N_LLD_FSA_MASK                (3)
#define     O1N_LLD_BSA_SHIFT               (8)
#define     O1N_LLD_OTP_SHIFT               (5)
#define     O1N_LLD_OTP_LOCK                (2)

#define     O1N_LLD_LOCK_SHIFT              (14)

/* OTP operation                */
#define     O1N_LLD_OTP_LOCK_MASK           (0xFF)
#define     O1N_LLD_OTP_PAGE_OFFSET         (0)
#define     O1N_LLD_LOCK_SPARE_BYTE_OFFSET  (14)

/* Buffer Sector Count specifies the number of sectors to read               */
#define     O1N_LLD_BSC00                   (0)

/* Buffer Sector Address specifies the sector address in the internal BootRAM and DataRAM */
#define     O1N_LLD_BSA1000                 (0x8)
#define     O1N_LLD_BSA1100                 (0xC)

/*****************************************************************************/
/* OneNAND System Configureation1 Register Values                            */
/*****************************************************************************/

#define     O1N_LLD_NUM_OF_SECT             (4)

#define     O1N_LLD_CPBK_SPARE              (2048)

/* with the help of emulator, LLD can run without the OneNAND
 * in that case, define FSR_ONENAND_EMULATOR
 */
#if defined (FSR_ONENAND_EMULATOR)

    #define     O1N_WRITE(nAddr, nDQ)                   {FSR_FOE_Write((UINT32)&(nAddr), nDQ);}
    #define     O1N_READ(nAddr)                         ((UINT16) FSR_FOE_Read((UINT32)&(nAddr)))
    #define     O1N_SET(nAddr, nDQ)                     {FSR_FOE_Write((UINT32)&(nAddr), (UINT16) FSR_FOE_Read((UINT32)&(nAddr)) | nDQ);}
    #define     O1N_CLR(nAddr, nDQ)                     {FSR_FOE_Write((UINT32)&(nAddr), (UINT16) FSR_FOE_Read((UINT32)&(nAddr)) & nDQ);}

    #define     O1N_TRANSTONAND(pDst, pSrc, nSize)                            \
                    FSR_FOE_TransferToDataRAM(pDst, pSrc, nSize)

    /* nSize is the number of bytes to transfer                               */
    #define     O1N_TRANSFROMNAND(pDst, pSrc, nSize)                          \
                    FSR_FOE_TransferFromDataRAM(pDst, pSrc, nSize)
#else /* #if defined (FSR_ONENAND_EMULATOR) */

    #define     O1N_WRITE(nAddr, nDQ)                   {nAddr  = nDQ;}
    #define     O1N_READ(nAddr)                         (nAddr        )
    #define     O1N_SET(nAddr, nDQ)                     {nAddr  = (nAddr | nDQ);}
    #define     O1N_CLR(nAddr, nDQ)                     {nAddr  = (nAddr & nDQ);}

    /* nSize is the number of bytes to transfer */
    #define     O1N_TRANSTONAND(pDst, pSrc, nSize)     O1N_PAM_TransToNAND(pDst, pSrc, nSize)
    #define     O1N_TRANSFROMNAND(pDst, pSrc, nSize)   O1N_PAM_TransFromNAND(pDst, pSrc, nSize)

#endif /* #if defined (FSR_ONENAND_EMULATOR) */

#define WAIT_O1N_INT_STAT(x, a)                        {while ((O1N_READ(x->nInt) & a) != (UINT16)a){}}

/*****************************************************************************/
/* Global variable definitions                                               */
/*****************************************************************************/
#define     O1N_LLD_CMD_LOAD                (0x0000)
#define     O1N_LLD_CMD_1X_PROGRAM          (0x0080)
#define     O1N_LLD_CMD_2X_PROGRAM          (0x007D)
#define     O1N_LLD_CMD_UNLOCK_BLOCK        (0x0023)
#define     O1N_LLD_CMD_LOCK_BLOCK          (0x002A)
#define     O1N_LLD_CMD_LOCKTIGHT_BLOCK     (0x002C)
#define     O1N_LLD_CMD_UNLOCK_ALLBLOCK     (0x0027)
#define     O1N_LLD_CMD_ERASE               (0x0094)
#define     O1N_LLD_CMD_RST_NFCORE          (0x00F0)
#define     O1N_LLD_CMD_HOT_RESET           (0x00F3)
#define     O1N_LLD_CMD_OTP_ACCESS          (0x0065)

#define     O1N_LLD_SECTOR_SIZE             (512)

/* write protection status register                                          */
#define     O1N_LLD_BLK_STAT_MASK           (0x0007)
#define     O1N_LLD_BLK_STAT_UNLOCKED       (0x0004)
#define     O1N_LLD_BLK_STAT_LOCKED         (0x0002)
#define     O1N_LLD_BLK_STAT_LOCKED_TIGHT   (0x0001)

/*****************************************************************************/
/* Extern variable declarations                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Local constant definitions                                                */
/*****************************************************************************/
/**
 * @brief data structure of OneNAND specification
 */
typedef struct
{
        UINT16      nMID;               /**< manufacturer ID                     */
        UINT16      nDID;               /**< device ID                           */
        
        UINT16      nNumOfBlks;         /**< the number of blocks                */
        UINT16      nNumOfBlksIn1stDie; /**< the number of blocks in first die   */
        UINT16      nNumOfPlanes;       /**< the number of planes                */
        UINT16      nSparePerSct;       /**< # of bytes of spare per sector      */

        UINT16      nSctsPerPG;         /**< the number of sectors per page      */
        UINT16      nNumOfRsvrInSLC;    /**< # of bad blocks in SLC area         */
        UINT32      nPgsPerBlkForSLC;   /**< # of pages per block in SLC area    */
        
        UINT16      nNumOfDies;         /**< # of dies in NAND device            */
        UINT16      nUserOTPScts;       /**< # of user sectors                   */
        BOOL32      b1stBlkOTP;         /**< support 1st block OTP or not        */
} OneNANDSpec;

/**
 * @brief data structure of OneNAND LLD context for each device number
 */
typedef struct
{
    UINT32       nBaseAddr;             /**< the base address of OneNAND    */
    BOOL32       bOpen;                 /**< open flag : TRUE32 or FALSE32       */

    UINT8        nDDPSelSft;            /**< the shift value of DDP selection    */
    UINT8        nFPASelSft;            /**< the shift value of FPA selection    */
    UINT16       nFBAMask;              /**< the mask of Flash Block Address     */

    UINT8        nFSAMask;              /**< the mask of Flash Sector Address    */
    UINT16       nBSASelSft;            /**< the shift value of BSA selection    */

    UINT16       nBlksForSLCArea[O1N_LLD_MAX_DIES];/**< # of blocks for SLC area     */

    UINT16       nSysConf1;             /**< when opening a device save the value
                                           of system configuration 1 register.
                                           restore this value after hot reset     */
    UINT16       nPad0;

    OneNANDSpec *pstONDSpec;            /**< pointer to OneNANDSpec              */

} OneNANDCxt;

PRIVATE OneNANDCxt     gstONDCxt;       /**< global structure of OneNAND Context */

/* OneNAND specification for OneNANDSpec. */
PRIVATE const OneNANDSpec gstONDSpec[] = {
/**********************************************************************/
/* 1. nMID                                                            */
/* 2.         nDID                                                    */
/* 3.                 nNumOfBlks                                      */
/* 4.                       nNumOfBlksIn1stDie                        */
/* 5.                             nNumOfPlanes                        */
/* 6.                                nSparePerSct                     */
/* 7.                                    nSctsPerPG                   */
/* 8.                                       nNumOfRsvrInSLC           */
/* 9.                                           nPgsPerBlkForSLC      */
/* 10.                                              nNumOfDies        */
/* 11.                                                 nUserOTPScts   */
/* 12.                                                     b1stBlkOTP */
/**********************************************************************/
    /* KFG1G16Q2M */
    { 0x00EC, 0x0030, 1024, 1024, 1, 16, 4, 20, 64, 1, 50, TRUE32},

    /* KFG2G16Q2M */
    { 0x00EC, 0x0044, 2048, 2048, 2, 16, 4, 40, 64, 1, 50, TRUE32},

    /* KFH4G16Q2M */
    { 0x00EC, 0x005C, 4096, 2048, 2, 16, 4, 80, 64, 2, 50, TRUE32},

    /* KFM2G16Q2M */
    { 0x00EC, 0x0040, 2048, 2048, 2, 16, 4, 40, 64, 1, 50, TRUE32},

    /* KFN2G16Q2A */
    { 0x00EC, 0x0048, 2048, 1024, 1, 16, 4, 40, 64, 2, 50, TRUE32},

    /* KFN4G16Q2M */
    { 0x00EC, 0x0058, 4096, 2048, 2, 16, 4, 40, 64, 2, 50, TRUE32},

#if defined (FSR_ONENAND_EMULATOR)
    /* for emulator */
    { 0x00EC, 0xF0FF,  512, 512,  2, 16, 4, 10, 64, 1, 50, TRUE32},

    { 0x00EC, 0xF0FE, 1024, 1024, 2, 16, 4, 20, 64, 1, 50, TRUE32},

#endif

    {      0,      0,    0,   0,  0,  0, 0,  0,  0, 0, 0 , 0},
};

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/

/**
 * @brief          this function opens OneNAND device driver
 *
 * @param[in]      nBaseAddr    : Base address of mapped DataRAM
 *
 * @return         O1N_LLD_RETURN_SUCCESS  
 * @return         
 * @return         
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark
 *
 */
PUBLIC INT32
O1N_LLD_Open(UINT32  nBaseAddr)
{
             OneNANDCxt        *pstONDCxt;
             OneNANDSpec       *pstONDSpec;
    volatile OneNANDReg        *pstONDReg;

             UINT32             nCnt;
             UINT32             nIdx;
             UINT32             nDie;
             
             INT32              nLLDRe    = O1N_LLD_RETURN_SUCCESS;

             UINT16             nMID;
             UINT16             nDID;
             UINT16             nVID;

             UINT16             nShifted;
             UINT16             nNumOfBlksIn2ndDie;
             UINT16             nPbn;

             /* Clear Buffer for DataRAM */
             UINT8              nClrBuf[2048] = {0xFF};

    do
    {
        pstONDCxt = &gstONDCxt;

        pstONDCxt->nBaseAddr = nBaseAddr;

        /* set base-address */
        pstONDReg = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

        nMID = O1N_READ(pstONDReg->nMID);
        nDID = O1N_READ(pstONDReg->nDID);
        nVID = O1N_READ(pstONDReg->nVerID);

        O1N_LLD_PRINT((TEXT("[O1N:INF]   nMID=0x%04x, nDID=0x%04x, nVID=0x%04x\r\n"),
            nMID, nDID, nVID));

        /* find current device */
        for (nCnt = 0; gstONDSpec[nCnt].nMID != 0; nCnt++)
        {
            if ((nMID == gstONDSpec[nCnt].nMID) &&
                (nDID == gstONDSpec[nCnt].nDID))
            {
                pstONDCxt->bOpen      = TRUE32;
                pstONDCxt->pstONDSpec = (OneNANDSpec *) &gstONDSpec[nCnt];
                break;
            }
            else
            {
                continue;
            }
        }

        if (pstONDCxt->bOpen != TRUE32)
        {
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        /* map spec. infomation */
        pstONDSpec = pstONDCxt->pstONDSpec;

        /*
         * Start Address8 Register 
         *     | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
         *     |=======================================|=============================|=========|
         *     |            Reserved                   |          FPA                |   FSA   |
         */
        pstONDCxt->nFPASelSft = O1N_LLD_FPA_SHIFT;
        pstONDCxt->nFSAMask   = O1N_LLD_FSA_MASK;
        /*
         * Start Buffer Register 
         *     | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
         *     |===================|===================|=============================|=========|
         *     |    Reserved       |       BSA         |        Reserved             |   BSC   |
         */
        pstONDCxt->nBSASelSft = O1N_LLD_BSA_SHIFT;

        /* set DBS                         */
        O1N_WRITE(pstONDReg->nStartAddr2, (UINT16) 0x00);

        /* 
         * the mask of Flash Block Address : all bit is set to '1'
         * If each device has different number of blocks in case of DDP, 
         * you shoud be considered condition about FBA mask.
         */
        nNumOfBlksIn2ndDie = pstONDSpec->nNumOfBlks - pstONDSpec->nNumOfBlksIn1stDie;
        if ((nNumOfBlksIn2ndDie != 0) &&
            (pstONDSpec->nNumOfBlksIn1stDie != nNumOfBlksIn2ndDie))
        {
            pstONDCxt->nFBAMask   = pstONDSpec->nNumOfBlksIn1stDie - 1;
        }
        else
        {
            pstONDCxt->nFBAMask   = (pstONDSpec->nNumOfBlks / pstONDSpec->nNumOfDies) - 1;
        }

        /* calculate nDDPSelSft */
        pstONDCxt->nDDPSelSft = 0;
        nShifted = pstONDCxt->nFBAMask << 1;
        while ((nShifted & O1N_LLD_MASK_DFS) != O1N_LLD_MASK_DFS)
        {
            pstONDCxt->nDDPSelSft++;
            nShifted <<= 1;
        }

        for (nDie = 0; nDie < pstONDSpec->nNumOfDies; nDie++)
        {
            pstONDCxt->nBlksForSLCArea[nDie] = pstONDSpec->nNumOfBlksIn1stDie;

            if (nDie != 0)
            {
                pstONDCxt->nBlksForSLCArea[nDie] = nNumOfBlksIn2ndDie;
            }

        }

        /* HOT-RESET initializes the value of register file, which includes 
         * system configuration 1 register.
         * for device to function properly, save the value of system
         * configuration 1 register at open time & restore it after HOT-RESET
         */
        pstONDCxt->nSysConf1       =  O1N_READ(pstONDReg->nSysConf1);

        /* clear DataRAM */
        for (nIdx = 0; nIdx < pstONDSpec->nNumOfDies; nIdx++)
        {
            nPbn = (UINT16) (pstONDSpec->nNumOfBlks / pstONDSpec->nNumOfDies * nIdx);

            /* set DBS                         */
            O1N_WRITE(pstONDReg->nStartAddr2, 
                      ((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);

            O1N_TRANSTONAND((VOID *) pstONDReg->nDataMB00, 
                            &nClrBuf[0],
                            pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE);
            O1N_TRANSTONAND((VOID *) pstONDReg->nDataMB10,
                            &nClrBuf[0],
                            pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE);

            O1N_TRANSTONAND((VOID *) pstONDReg->nDataSB00,
                            &nClrBuf[0],
                            pstONDSpec->nSctsPerPG * pstONDSpec->nSparePerSct);
            O1N_TRANSTONAND((VOID *) pstONDReg->nDataSB10,
                            &nClrBuf[0],
                            pstONDSpec->nSctsPerPG * pstONDSpec->nSparePerSct);
        }
    } while (0);

    return(nLLDRe);
}

/**
 * @brief          This function closes OneNAND device driver
 *
 * @param[in]      nono
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark
 *
 */
PUBLIC INT32
O1N_LLD_Close(VOID)
{
    OneNANDCxt       *pstONDCxt  = &gstONDCxt;

    INT32             nLLDRe     = O1N_LLD_RETURN_SUCCESS;

    do
    {
        pstONDCxt->bOpen      = FALSE32;
        pstONDCxt->pstONDSpec = NULL;

    } while (0);

    return(nLLDRe);
}



/**
 * @brief          this function writes data into NAND flash
 *
 * @param[in]      nPbn         : Physical Block  Number
 * @param[in]      nPgOffset    : Page Offset within a block
 * @param[in]      pMBuf        : Memory buffer for main  array of NAND flash
 * @param[in]      pSBuf        : Memory buffer for spare array of NAND flash
 * @param[in]      nFlag        : Operation options to select 1x or 2x program / DataRAM0 or DataRAM1
 *                                O1N_LLD_FLAG_1X_PROGRAM
 *                                O1N_LLD_FLAG_2X_PROGRAM
 *                                O1N_LLD_FLAG_USE_DATARAM0
 *                                O1N_LLD_FLAG_USE_DATARAM1
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR |
 *                 {O1N_LLD_1STPLN_ERROR | O1N_LLD_2NDPLN_ERROR}
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         
 *
 */
PUBLIC INT32
O1N_LLD_Write(UINT32       nPbn,
              UINT32       nPgOffset,
              UINT8       *pMBuf,
              UINT8       *pSBuf,
              UINT32       nFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
             OneNANDSpec      *pstONDSpec = pstONDCxt->pstONDSpec;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;
             
             UINT32            nDie;

             UINT32            nPairedPbn;
             UINT32            nUseBuf = 0;
             INT32             nLLDRe       = O1N_LLD_RETURN_SUCCESS;
             UINT16            nBSA         = 0;

             UINT16            nWrProtectStat;

    do
    {
        nDie    = (nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft)) & 0x1;

        /* set DBS                         */
        O1N_WRITE(pstONDReg->nStartAddr2, 
                  ((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);

        if (nFlag & O1N_LLD_FLAG_USE_DATARAM1)
        {
            nUseBuf = 1;
        }

        if (pMBuf != NULL)
        {
            /* when write program is under way, BSA to DataRAM is always set to 1000 */
            O1N_TRANSTONAND(&(pstONDReg->nDataMB00[0]) + 
                            (nUseBuf & 1) * pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE,
                            pMBuf,
                            pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE);

            /* consider about 2xPGM */
            if (nFlag & O1N_LLD_FLAG_2X_PROGRAM)
            {
                /* Use second buffer - DataRAM10 */
                O1N_TRANSTONAND(&(pstONDReg->nDataMB10[0]), 
                                pMBuf + pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE, 
                                pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE);
            }
        } /* if (pMBuf != NULL) */

        if (pSBuf != NULL)
        {
            /* when write program is under way, BSA to DataRAM is always set to 1000 */
            O1N_TRANSTONAND(&(pstONDReg->nDataSB00[0]) +
                            (nUseBuf & 1) * pstONDSpec->nSctsPerPG * O1N_LLD_SPARE_SIZE,
                            pSBuf,
                            pstONDSpec->nSctsPerPG * O1N_LLD_SPARE_SIZE);

            /* consider about 2xPGM */
            if (nFlag & O1N_LLD_FLAG_2X_PROGRAM)
            {
                /* Use second buffer - DataRAM10 */
                O1N_TRANSTONAND(&(pstONDReg->nDataSB10[0]), 
                                pSBuf + pstONDSpec->nSctsPerPG * O1N_LLD_SPARE_SIZE, 
                                pstONDSpec->nSctsPerPG * O1N_LLD_SPARE_SIZE);
            }
        } /* if (pSBuf != NULL) */

        /* in case of 2 plane program, write protection have to be checked about second plane */
        if (nFlag & O1N_LLD_FLAG_2X_PROGRAM)
        {
            /* Set paired block number */
            nPairedPbn = nPbn + 1;

            /* set DFS, FBA                    */
            O1N_WRITE(pstONDReg->nStartAddr1, 
                      (((UINT16) nPairedPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) | 
                       ((UINT16) nPairedPbn & pstONDCxt->nFBAMask));

            nWrProtectStat = O1N_READ(pstONDReg->nWrProtectStat);
            if ((nWrProtectStat & O1N_LLD_BLK_STAT_MASK) != 
                O1N_LLD_BLK_STAT_UNLOCKED)
            {
                O1N_LLD_PRINT((TEXT("[O1N:ERR]   nPbn:%d is not unlocked\r\n"), nPairedPbn));
                O1N_LLD_PRINT((TEXT("            lock status: 0x%04x\r\n"), O1N_READ(pstONDReg->nWrProtectStat)));
                nLLDRe |= (O1N_LLD_RETURN_ERROR | 
                           O1N_LLD_RETURN_2NDPLN_ERROR);
            }
        }

        /* write bad mark */
        if (nUseBuf & 1)
        {
            O1N_WRITE(pstONDReg->nDataSB10[0], 
                      O1N_LLD_VALID_BLK_MARK);
        }
        else
        {
            O1N_WRITE(pstONDReg->nDataSB00[0], 
                      O1N_LLD_VALID_BLK_MARK);
        }

        /* set DFS, FBA                    */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                  (((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) | 
                   ((UINT16) nPbn & pstONDCxt->nFBAMask));

        /* write protection can be checked when DBS & FBA is set */
        nWrProtectStat = O1N_READ(pstONDReg->nWrProtectStat);
        if ((nWrProtectStat & O1N_LLD_BLK_STAT_MASK) != 
            O1N_LLD_BLK_STAT_UNLOCKED)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   nPbn:%d is not unlocked\r\n"), nPbn));
            O1N_LLD_PRINT((TEXT("            lock status: 0x%04x\r\n"), O1N_READ(pstONDReg->nWrProtectStat)));
            nLLDRe = (O1N_LLD_RETURN_ERROR | 
                      O1N_LLD_RETURN_1STPLN_ERROR);
        }

        /* if the write protection error is detected, break do-while(0) loop */
        if (nLLDRe & (O1N_LLD_RETURN_1STPLN_ERROR | 
                      O1N_LLD_RETURN_2NDPLN_ERROR))
        {
            break;
        }

        /* set Start Page Address (FPA)    */
        O1N_WRITE(pstONDReg->nStartAddr8, 
                 (UINT16) nPgOffset << pstONDCxt->nFPASelSft);

        /* set Start Buffer Register (BSA & BSC) */
        /* BSA should be considered to use each buffer - DataRAM00 and DataRAM10 */
        nBSA = O1N_LLD_BSA1000;

        if (nFlag & O1N_LLD_FLAG_USE_DATARAM1)
        {
            nBSA = O1N_LLD_BSA1100;
        }

        /* Set to using number of sectors */
        O1N_WRITE(pstONDReg->nStartBuf, 
                 (nBSA << pstONDCxt->nBSASelSft) | O1N_LLD_BSC00);

        O1N_CLR(pstONDReg->nSysConf1, O1N_LLD_CONF1_ECC_ON);

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* issue command */
        if (!(nFlag & O1N_LLD_FLAG_2X_PROGRAM))
        {
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_1X_PROGRAM);
        }
        else
        {
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_2X_PROGRAM);
        }

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        /* Read Controller Status Register */
        if (O1N_READ (pstONDReg->nCtrlStat) & O1N_LLD_STATUS_ERROR)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   Program error at nPbn:%d, nPgOffset:%d\r\n"), nPbn, nPgOffset));
            nLLDRe = O1N_LLD_RETURN_ERROR;
        }
    } while (0);

    return(nLLDRe);
}

/**
 * @brief          this function writes data into NAND flash
 *
 * @param[in]      nPbn         : Physical Block  Number
 * @param[in]      nPgOffset    : Page Offset within a block
 * @param[in]      pMBuf        : Memory buffer for main  array of NAND flash
 * @param[in]      pSBuf        : Memory buffer for spare array of NAND flash
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR |
 *                 {O1N_LLD_1STPLN_ERROR | O1N_LLD_2NDPLN_ERROR}
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         
 *
 */
PUBLIC INT32
O1N_LLD_WriteOTP(UINT32       nPbn,
                 UINT32       nPgOffset,
                 UINT8       *pMBuf,
                 UINT8       *pSBuf)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;
             
             UINT32            nDie;

             INT32             nLLDRe       = O1N_LLD_RETURN_SUCCESS;

    do
    {
        nDie    = (nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft)) & 0x1;

        if (nPbn != 0)
        {
            O1N_LLD_PRINT((TEXT("[O1N:Err]   Block : %d - nPbn should be 0.\r\n"), nPbn));
            break;
        }

        /* set DBS                         */
        O1N_WRITE(pstONDReg->nStartAddr2, 
                  ((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);

        /* set DFS, FBA                    */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                  (((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) | 
                   ((UINT16) nPbn & pstONDCxt->nFBAMask));

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* Write 'OTP Access' command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_OTP_ACCESS);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        nLLDRe = O1N_LLD_Write(nPbn, 
                               nPgOffset, 
                               pMBuf, 
                               pSBuf, 
                               O1N_LLD_FLAG_1X_PROGRAM | 
                               O1N_LLD_FLAG_USE_DATARAM0);

        if (nLLDRe != O1N_LLD_RETURN_SUCCESS)
        {
            O1N_LLD_PRINT((TEXT("[O1N:Err]   OTP Write failed.\r\n")));
            return nLLDRe = O1N_LLD_RETURN_ERROR;
        }

        /* issue command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_HOT_RESET);

        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        O1N_WRITE(pstONDReg->nSysConf1, pstONDCxt->nSysConf1);
    } while (0);

    return(nLLDRe);
}


/**
 * @brief          this function reads data from NAND flash
 *
 * @param[in]      nPbn         : Physical Block  Number
 * @param[in]      nPgOffset    : Page Offset within a block
 * @param[out]     pMBuf        : Memory buffer for main  array of NAND flash
 * @param[out]     pSBuf        : Memory buffer for spare array of NAND flash
 * @param[in]      nFlag        : O1N_LLD_FLAG_USE_DATARAM0
 *                                O1N_LLD_FLAG_USE_DATARAM1
 * 
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         
 *
 */
PUBLIC INT32
O1N_LLD_Read(UINT32       nPbn,
             UINT32       nPgOffset,
             UINT8       *pMBuf,
             UINT8       *pSBuf,
             UINT32       nFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
             OneNANDSpec      *pstONDSpec = pstONDCxt->pstONDSpec;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

             UINT32            nDie     = 0;
             UINT32            nUseBuf  = 0;

             INT32             nLLDRe     = O1N_LLD_RETURN_SUCCESS;
             UINT16            nBSA = 0;

     nDie    = (nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft)) & 0x1;

    do
    {
        /* set DBS */
        O1N_WRITE(pstONDReg->nStartAddr2, 
         ((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);

        /* set DFS & FBA */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                  (((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) |
                   ((UINT16) nPbn & pstONDCxt->nFBAMask));

        /* set FPA (Flash Page Address) */
        O1N_WRITE(pstONDReg->nStartAddr8, 
                 (UINT16) (nPgOffset << pstONDCxt->nFPASelSft));


        /* set BSA (Buffer Sector Address) & BSC (Buffer Sector Count) */
        /* BSA has to be set to 1000 */
        nBSA = O1N_LLD_BSA1000;

        if (nFlag & O1N_LLD_FLAG_USE_DATARAM1)
        {
            nBSA = O1N_LLD_BSA1100;
            nUseBuf = 1;
        }

        O1N_WRITE(pstONDReg->nStartBuf, 
                 (nBSA << pstONDCxt->nBSASelSft) | O1N_LLD_BSC00);

        /* Write System Configuration Register */
        O1N_CLR(pstONDReg->nSysConf1, O1N_LLD_CONF1_ECC_ON);

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* Write 'Load' command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_LOAD);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        if (O1N_READ (pstONDReg->nCtrlStat) == O1N_LLD_STATUS_ERROR)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   ECC Status : Uncorrectable at nPbn:%d, nPgOffset:%d\r\n"), nPbn, nPgOffset));
            nLLDRe = O1N_LLD_RETURN_ERROR;
        }

        /* Host reads data from DataRAM */
        if (pMBuf != NULL)
        {
            O1N_TRANSFROMNAND((VOID *) pMBuf,
                              (volatile VOID *) (&(pstONDReg->nDataMB00[0]) + 
                              (nUseBuf & 1) * pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE),
                              pstONDSpec->nSctsPerPG * O1N_LLD_SECTOR_SIZE);
        } /* if (pMBuf != NULL) */

        if (pSBuf != NULL)
        {
            O1N_TRANSFROMNAND((VOID *) pSBuf,
                              (volatile VOID *) (&(pstONDReg->nDataSB00[0]) + 
                              (nUseBuf & 1) * pstONDSpec->nSctsPerPG * O1N_LLD_SPARE_SIZE),
                              pstONDSpec->nSctsPerPG * O1N_LLD_SPARE_SIZE);
        } /* if (pSBuf != NULL) */

    } while (0);

    return(nLLDRe);
}

/**
 * @brief          this function reads data from OTP block 
 *
 * @param[in]      nPbn         : Physical Block  Number
 * @param[in]      nPgOffset    : Page Offset within a block
 * @param[out]     pMBuf        : Memory buffer for main  array of NAND flash
 * @param[out]     pSBuf        : Memory buffer for spare array of NAND flash
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         
 *
 */
PUBLIC INT32
O1N_LLD_ReadOTP(UINT32       nPbn,
                UINT32       nPgOffset,
                UINT8       *pMBuf,
                UINT8       *pSBuf)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

             UINT32            nDie = 0;

             INT32             nLLDRe     = O1N_LLD_RETURN_SUCCESS;

     nDie    = (nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft)) & 0x1;

    do
    {
        /* set DBS */
        O1N_WRITE(pstONDReg->nStartAddr2, 
         ((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);

        /* set DFS & FBA */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                  (((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) |
                   ((UINT16) nPbn & pstONDCxt->nFBAMask));

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* Write 'OTP Access' command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_OTP_ACCESS);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        nLLDRe = O1N_LLD_Read(nPbn, 
                              nPgOffset, 
                              pMBuf, 
                              pSBuf, 
                              O1N_LLD_FLAG_USE_DATARAM0);

        if (nLLDRe != O1N_LLD_RETURN_SUCCESS)
        {
            O1N_LLD_PRINT((TEXT("[O1N:Err]   OTP Read failed.\r\n")));
            return nLLDRe = O1N_LLD_RETURN_ERROR;
        }

        /* Write 'Load' command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_HOT_RESET);

        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        O1N_WRITE(pstONDReg->nSysConf1, pstONDCxt->nSysConf1);
    } while (0);

    return(nLLDRe);
}


/**
 * @brief          this function erase block in NAND flash
 *
 * @param[in]      nPbn         : block number
 *
 * @return         O1N_LLD_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         v2.0.0 supports only single plane, one block erase
 *
 */
PUBLIC INT32
O1N_LLD_Erase(UINT32  nPbn)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

             INT32             nLLDRe     = O1N_LLD_RETURN_SUCCESS;
             UINT32            nDie;
             UINT16            nWrProtectStat;

    do
    {
        nDie = nPbn >> (O1N_LLD_DBS_BASEBIT - pstONDCxt->nDDPSelSft);

        /* set DFS, FBA                   */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                 (UINT16) ((nDie << O1N_LLD_DBS_BASEBIT) | (nPbn & pstONDCxt->nFBAMask)));

        /* set DBS                        */
        O1N_WRITE(pstONDReg->nStartAddr2, 
                 (UINT16) (nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS) ;

        /* write protection can be checked at this point */
        nWrProtectStat = O1N_READ(pstONDReg->nWrProtectStat);
        if ((nWrProtectStat & O1N_LLD_BLK_STAT_MASK) != O1N_LLD_BLK_STAT_UNLOCKED)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   nPbn:%d is not unlocked\r\n"), nPbn));
            O1N_LLD_PRINT((TEXT("            lock status: 0x%04x\r\n"), O1N_READ(pstONDReg->nWrProtectStat)));
            nLLDRe = (O1N_LLD_RETURN_ERROR | O1N_LLD_RETURN_1STPLN_ERROR);
            break;
        }

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_ERASE);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        /* Read Controller Status Register */
        if (O1N_READ (pstONDReg->nCtrlStat) == O1N_LLD_STATUS_ERROR)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   Erase error at nPbn:%d\r\n"), nPbn));
            nLLDRe = O1N_LLD_RETURN_ERROR;
        }

    } while (0);

    return(nLLDRe);
}


/**
 * @brief          This function checks a bad block
 *
 * @param[in]      nPbn         : physical block number
 * @param[in]      nFlag        : find the operation whether 1x or 2x
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_INIT_BADBLOCK
 * @return         O1N_LLD_RETURN_1STPLN_ERROR | O1N_LLD_RETURN_2NDPLN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         
 *
 */
PUBLIC INT32
O1N_LLD_ChkBadBlk(UINT32 nPbn,
                  UINT32 nFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

             INT32             nLLDRe = O1N_LLD_RETURN_SUCCESS;
             UINT16            nDQ[4];
             UINT32            nPpn;
             UINT32            nPairedBlk;

    do
    {

        /* Check "FFFF" at the 1st word of sector 0 of spare area in 1st and 2nd page */
        for (nPpn = 0; nPpn < 2; nPpn++)
        {
            O1N_LLD_Read(nPbn,                                                   /* Physical Block Number         */
                        (UINT32) nPpn,                                          /* page offset to be read        */
                        (UINT8 *) NULL,                                         /* Buffer pointer for Main area  */
                        (UINT8 *) NULL,                                         /* Buffer pointer for Spare area */
                        O1N_LLD_FLAG_USE_DATARAM0);

            nDQ[nPpn] = O1N_READ(*(volatile UINT16 *) (&pstONDReg->nDataSB00[0]));

        }

        if (nFlag & O1N_LLD_FLAG_2X_CHK_BADBLOCK)
        {
            nPairedBlk = nPbn + 1;

            /* Check "FFFF" at the 1st word of sector 0 of spare area in 1st and 2nd page */
            for (nPpn = 0; nPpn < 2; nPpn++)
            {
                O1N_LLD_Read(nPairedBlk,                                                   /* Physical Block Number         */
                            (UINT32) nPpn,                                          /* page offset to be read        */
                            (UINT8 *) NULL,                                         /* Buffer pointer for Main area  */
                            (UINT8 *) NULL,                                         /* Buffer pointer for Spare area */
                            O1N_LLD_FLAG_USE_DATARAM0);

                nDQ[nPpn + 2] = O1N_READ(*(volatile UINT16 *) (&pstONDReg->nDataSB00[0]));
            }

            /* if one of them is not "FFFF", return error */
            if ((nDQ[2] != (UINT16)O1N_LLD_VALID_BLK_MARK) || 
                (nDQ[3] != (UINT16)O1N_LLD_VALID_BLK_MARK))
            {
                O1N_LLD_PRINT((TEXT("\r\n[O1N:INF]   nPbn:%d is bad block\r\n"), nPairedBlk));
                O1N_LLD_PRINT((TEXT("            bad mark: 0x%04x\r\n"), nDQ));
                nLLDRe = O1N_LLD_RETURN_2NDPLN_ERROR | O1N_LLD_INIT_BADBLOCK;
            }
        }

        /* if one of them is not "FFFF", return error */
        if ((nDQ[0] != (UINT16)O1N_LLD_VALID_BLK_MARK) || 
            (nDQ[1] != (UINT16)O1N_LLD_VALID_BLK_MARK))
        {
            O1N_LLD_PRINT((TEXT("\r\n[O1N:INF]   nPbn:%d is bad block\r\n"), nPbn));
            O1N_LLD_PRINT((TEXT("            bad mark: 0x%04x\r\n"), nDQ));
            nLLDRe = O1N_LLD_RETURN_1STPLN_ERROR | O1N_LLD_INIT_BADBLOCK;
            break;
        }
    } while (0);

    return(nLLDRe);
}

/**
 * @brief          this function reports device information to upper layer.
 *
 * @param[out]     pstONDSpec   : pointer to the device spec
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark
 *
 */
PUBLIC INT32
O1N_LLD_GetDevSpec(Open1NSpec *pstDevSpec)
{
    OneNANDCxt  *pstONDCxt  = &gstONDCxt;
    OneNANDSpec *pstONDSpec = pstONDCxt->pstONDSpec;

    INT32        nLLDRe     = O1N_LLD_RETURN_SUCCESS;
    UINT32       nDieIdx;

    do 
    {

        O1N_OAM_MEMSET(pstDevSpec, 0xFF, sizeof(Open1NSpec));

        pstDevSpec->nNumOfBlks                      = pstONDSpec->nNumOfBlks;
        pstDevSpec->nNumOfPlanes                    = pstONDSpec->nNumOfPlanes;
        pstDevSpec->nNumOfBlksIn1stDie              = pstONDSpec->nNumOfBlksIn1stDie;
        pstDevSpec->nDID                            = pstONDSpec->nDID;

        for (nDieIdx = 0; nDieIdx < pstONDSpec->nNumOfDies; nDieIdx++)
        {
            pstDevSpec->nBlksForSLCArea[nDieIdx]    = pstONDCxt->nBlksForSLCArea[nDieIdx];
        }

        pstDevSpec->nSctsPerPG                      = pstONDSpec->nSctsPerPG;
        pstDevSpec->nSparePerSct                    = pstONDSpec->nSparePerSct;
        pstDevSpec->nPgsPerBlkForSLC                = pstONDSpec->nPgsPerBlkForSLC;
        pstDevSpec->nPgsPerBlkForMLC                = 0;
        pstDevSpec->nNumOfDies                      = pstONDSpec->nNumOfDies;

        pstDevSpec->nRsvBlksInDev                   = pstONDSpec->nNumOfRsvrInSLC;

        pstDevSpec->nNANDType                       = O1N_LLD_SLC_ONENAND;

    } while (0);

    return(nLLDRe);
}

/**
 * @brief          this function changes the lock status of the block
 *
 * @param[in]      nPbn      : Physical Block Number
 * @param[in]      nType     : { O1N_LLD_UNLOCK_BLOCK | O1N_LLD_UNLOCK_ALL_BLOCK |
 * @n                            O1N_LLD_LOCK_BLOCK   | O1N_LLD_LOCK_TIGHT_BLOCK }
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @n              O1N_LLD_RETURN_ERROR
 *
 * @since          since v2.0.0
 *
 */
PUBLIC INT32   
O1N_LLD_ChangeLockStat(UINT32   nPbn,
                       UINT32   nType)
{
             OneNANDCxt        *pstONDCxt = &gstONDCxt;
    volatile OneNANDReg        *pstONDReg = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

             INT32              nLLDRe = O1N_LLD_RETURN_SUCCESS;

             UINT32             nDie;
             UINT32             nFBA;

    do
    {
        nDie = nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft);
        nFBA = nPbn & pstONDCxt->nFBAMask;

        /* Write 'DFS' of Flash */
        O1N_WRITE(pstONDReg->nStartAddr1, (UINT16) (nDie << O1N_LLD_DFS_BASEBIT));

        /* Select DataRAM for DDP */
        O1N_WRITE(pstONDReg->nStartAddr2, (UINT16) (nDie << O1N_LLD_DBS_BASEBIT));

        /* Write 'SBA' of Flash */
        O1N_WRITE(pstONDReg->nStartBlkAddr, (UINT16) nFBA);

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* Write 'lock/unlock/lock-tight' command */
        switch (nType)
        {
        case O1N_LLD_FLAG_UNLOCK_BLOCK:
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_UNLOCK_BLOCK);
            break;

        case O1N_LLD_FLAG_UNLOCK_ALL_BLOCK:
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_UNLOCK_ALLBLOCK);
            break;

        case O1N_LLD_FLAG_LOCK_BLOCK:
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_LOCK_BLOCK);
            break;

        case O1N_LLD_FLAG_LOCK_TIGHT_BLOCK:
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_LOCKTIGHT_BLOCK);
            break;

        default:
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   Irregal Flag\r\n")));
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        /* wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        /* read controller status register */
        if (O1N_READ (pstONDReg->nCtrlStat) == O1N_LLD_STATUS_ERROR)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   fail changing lock status of Pbn:%d\r\n"), nPbn));

            /* Write 'DFS' of Flash */
            O1N_WRITE(pstONDReg->nStartAddr1, (UINT16) ((nDie << O1N_LLD_DFS_BASEBIT) | nFBA));

            /* Select DataRAM for DDP */
            O1N_WRITE(pstONDReg->nStartAddr2, (UINT16) (nDie << O1N_LLD_DBS_BASEBIT));

            O1N_LLD_PRINT((TEXT("            lock status: 0x%04x\r\n"), O1N_READ(pstONDReg->nWrProtectStat)));
            nLLDRe = O1N_LLD_RETURN_ERROR;
        }

        /* lock/unlock/lock-tight completed */
    } while (0);

    return nLLDRe;
}

/**
 * @brief           this function locks the OTP block, 1st OTP block
 *
 * @param[in]       nLockFlag   : this is programmed into 1st word of sector0 of
 * @n                             main of page0 in the OTP block
 *                                O1N_LLD_FLAG_LOCK_OTP_BLOCK    
 *                                O1N_LLD_FLAG_LOCK_1ST_BLOCK_OTP
 *                                O1N_LLD_FLAG_LOCK_BOTH_OTP     
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author          JeongWook Moon
 * @version         2.0.0
 * @remark
 *
 */
PUBLIC INT32
O1N_LLD_ProtectOTP(UINT32 nLockFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
             OneNANDSpec      *pstONDSpec = pstONDCxt->pstONDSpec;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

            UINT16              nRegVal;
            UINT32              nLockStat;
            UINT32              nPbn;

            INT32               nLLDRe     = O1N_LLD_RETURN_SUCCESS;
            UINT16              nLockValue;

    do
    {
        /* STEP 1 : check the control status register to confirm the lock bit */
        nRegVal = (O1N_READ(pstONDReg->nCtrlStat) & O1N_LLD_MASK_OTP) >> O1N_LLD_OTP_SHIFT;

        /*
         * Controller Status Register 
         *     | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
         *     |====|====|====|====|====|====|====|====|====|====|====|====|====|====|====|====|
         *     |OnGo|Lock|Load|Prog|Erse| Err| Sus| Rsv|RSTB|OTP |OTP |Prv1|Cur1|Prv2|Cur2| TO |
         *                                                   (l)  (BL)      
         */
        if (nRegVal == 0x01)
        {
            nLockStat = O1N_LLD_OTP_OTP_BLK_UNLKED | 
                        O1N_LLD_OTP_1ST_BLK_LOCKED;
        }
        else if (nRegVal == 0x02)
        {
            nLockStat = O1N_LLD_OTP_OTP_BLK_LOCKED | 
                        O1N_LLD_OTP_1ST_BLK_UNLKED;
        }
        else if (nRegVal == 0x03)
        {
            nLockStat = O1N_LLD_OTP_OTP_BLK_LOCKED | 
                        O1N_LLD_OTP_1ST_BLK_LOCKED;
        }
        else
        {
            nLockStat = O1N_LLD_OTP_OTP_BLK_UNLKED | 
                        O1N_LLD_OTP_1ST_BLK_UNLKED;
        }
        

        /* STEP 2 : check the nLockFlag to lock selected OTP block */
        if ((nLockFlag & O1N_LLD_OTP_LOCK_MASK) == O1N_LLD_FLAG_LOCK_1ST_BLOCK_OTP)
        {
            /* it's an Error to lock 1st block as OTP
             * when this device doesn't use 1st block as OTP
             */
            if ((pstONDSpec->b1stBlkOTP) == FALSE32)
            {
                O1N_LLD_PRINT((TEXT("[O1N:ERR]   Invalid param. There is no supported 1st block OTP\r\n")));
                nLLDRe = O1N_LLD_RETURN_ERROR;
                break;
            }

            /* when OTP block is locked,
             * 1st block cannot be locked as OTP
             */
            if (nLockStat & O1N_LLD_OTP_OTP_BLK_LOCKED)
            {
                O1N_LLD_PRINT((TEXT("[O1N:ERR]   OTP block is aready locked/r/n")));
                nLLDRe = O1N_LLD_RETURN_ERROR;
                break;
            }
        }

        /* Set nPbn to 0 */
        nPbn = 0;

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* Write 'OTP Access' command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_OTP_ACCESS);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);


        nLLDRe = O1N_LLD_Read(nPbn,
                              O1N_LLD_OTP_PAGE_OFFSET,
                              NULL,
                              NULL,
                              O1N_LLD_FLAG_USE_DATARAM0);

        if (nLLDRe != O1N_LLD_RETURN_SUCCESS)
        {
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        /* modify both of spare area 
           because we don't known whether FSR_OND_Write() uses nDataSB00 or nDataSB10 */
        nLockValue = O1N_READ(*(UINT16 *) &pstONDReg->nDataSB00[O1N_LLD_LOCK_SPARE_BYTE_OFFSET]);
        nLockValue = nLockValue & (UINT16) (0xFF00 | nLockFlag);
        O1N_WRITE(*(UINT16 *) &pstONDReg->nDataSB00[O1N_LLD_LOCK_SPARE_BYTE_OFFSET], nLockValue);

        nLLDRe = O1N_LLD_Write(nPbn,
                               O1N_LLD_OTP_PAGE_OFFSET,
                               NULL,
                               NULL,
                               O1N_LLD_FLAG_USE_DATARAM0);

        if (nLLDRe != O1N_LLD_RETURN_SUCCESS)
        {
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        nLockValue |= 0xFF;
        O1N_WRITE(*(UINT16 *) &pstONDReg->nDataSB00[O1N_LLD_LOCK_SPARE_BYTE_OFFSET], nLockValue);

    } while (0);

    /* Exit OTP block */
    O1N_LLD_Reset(O1N_LLD_FLAG_NCORE_RESET);

    return nLLDRe;
}

/**
 * @brief          this function reads data from NAND flash, program with random data input
 *
 * @param[in]      pstCpArg     : pointer to the structure Open1NCpBkArg 
 * @param[in]      nFlag        : O1N_LLD_FLAG_USE_DATARAM0
 *                                O1N_LLD_FLAG_USE_DATARAM1
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark         
 *
 */
PUBLIC INT32
O1N_LLD_CopyBack(Open1NCpBkArg *pstCpArg,
                 UINT32         nFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;


             Open1NRndInArg   *pstRIArg; /* random in argument */

             UINT32            nCnt;
             UINT32            nPgOffset;
             UINT32            nUseBuf = 0;

             INT32             nLLDRe     = O1N_LLD_RETURN_SUCCESS;
             UINT32            nDie = 0;

             UINT16            nPbn;
             UINT16            nBSA = 0;

    do
    {
        /* Does not support 2x operation */
        if (nFlag & O1N_LLD_FLAG_2X_PROGRAM)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   Invalid param. There is no supported 2x CopyBack\r\n")));
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        /* load phase of copyback() checks the source block & page offset */
        nPbn      = pstCpArg->nSrcPbn;
        nPgOffset = pstCpArg->nSrcPgOffset;

        nDie = (nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft)) & 0x1;

        O1N_WRITE(pstONDReg->nStartAddr2, 
                 ((UINT16) (nDie & 1) << O1N_LLD_DBS_BASEBIT) & O1N_LLD_MASK_DBS) ;

        /* set DFS & FBA                 */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                  (((nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) | 
                   (nPbn & pstONDCxt->nFBAMask)));

        /* set FPA (Flash Page Address)  */
        O1N_WRITE(pstONDReg->nStartAddr8, 
                 (UINT16) nPgOffset << pstONDCxt->nFPASelSft);

        /* set Start Buffer Register (BSA & BSC) */
        /* BSA should be considered to use each buffer - DataRAM00 and DataRAM10 */
        nBSA = O1N_LLD_BSA1000;

        if (nFlag & O1N_LLD_FLAG_USE_DATARAM1)
        {
            nBSA    = O1N_LLD_BSA1100;
            nUseBuf = 1;
        }

        /* Set to using number of sectors */
        O1N_WRITE(pstONDReg->nStartBuf, 
                 (nBSA << pstONDCxt->nBSASelSft) | O1N_LLD_BSC00);

        /* Write System Configuration Register */
        O1N_CLR(pstONDReg->nSysConf1, O1N_LLD_CONF1_ECC_ON);

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* Write 'Load' command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_LOAD);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        if (O1N_READ (pstONDReg->nCtrlStat) == O1N_LLD_STATUS_ERROR)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   ECC Status : Uncorrectable at nPbn:%d, nPgOffset:%d\r\n"), nPbn, nPgOffset));
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        nPbn      = pstCpArg->nDstPbn;
        nPgOffset = pstCpArg->nDstPgOffset;

        nDie = (nPbn >> (O1N_LLD_DFS_BASEBIT - pstONDCxt->nDDPSelSft)) & 0x1;

        /* set DBS                      */
        O1N_WRITE(pstONDReg->nStartAddr2, 
                 (nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);


        /* if the spare buffer random-in is avaible, return error */
        for (nCnt = 0; nCnt < pstCpArg->nRndInCnt; nCnt++)
        {
            pstRIArg = pstCpArg->pstRndInArg + nCnt;

            /* in case copyback of spare area is requested */
            if (pstRIArg->nOffset >= O1N_LLD_CPBK_SPARE)
            {
                O1N_TRANSTONAND(&pstONDReg->nDataSB00[0] + 
                                (nUseBuf & 1) * O1N_LLD_SPARE_SIZE * pstONDCxt->pstONDSpec->nSctsPerPG + 
                                pstRIArg->nOffset - O1N_LLD_CPBK_SPARE, 
                                (UINT8 *) pstRIArg->pBuf, 
                                pstRIArg->nNumOfBytes);
            } /* if (pstRIArg->nOffset >= FSR_LLD_CPBK_SPARE) */
            else
            {
                O1N_TRANSTONAND(&pstONDReg->nDataMB00[0] + 
                                (nUseBuf & 1) * O1N_LLD_SECTOR_SIZE * pstONDCxt->pstONDSpec->nSctsPerPG + 
                                pstRIArg->nOffset, 
                                (UINT8 *) pstRIArg->pBuf, 
                                pstRIArg->nNumOfBytes);
            }
        } /* for (nCnt = 0; nCnt < pstCpArg->nRndInCnt; nCnt++) */

        /* set DFS & FBA                */
        O1N_WRITE(pstONDReg->nStartAddr1, 
                  (((nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) | 
                   (nPbn & pstONDCxt->nFBAMask)));

        /* write protection can be checked at this point */
        if ((O1N_READ (pstONDReg->nWrProtectStat) & O1N_LLD_BLK_STAT_MASK) != O1N_LLD_BLK_STAT_UNLOCKED)
        {
            O1N_LLD_PRINT((TEXT("[OND:ERR]   Pbn is write protected\r\n")));
            nLLDRe = O1N_LLD_RETURN_ERROR;
            break;
        }

        /* set FPA (Flash Page Address)  */
        O1N_WRITE(pstONDReg->nStartAddr8, 
                 (UINT16) nPgOffset << pstONDCxt->nFPASelSft);

        O1N_CLR(pstONDReg->nSysConf1, O1N_LLD_CONF1_ECC_ON);

        /* Write 0 to interrupt register */
        O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

        /* issue command */
        O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_1X_PROGRAM);

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        /* Read Controller Status Register */
        if (O1N_READ (pstONDReg->nCtrlStat) == O1N_LLD_STATUS_ERROR)
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   Program error at nPbn:%d, nPgOffset:%d\r\n"), nPbn, nPgOffset));
            nLLDRe = O1N_LLD_RETURN_ERROR;
        }
    } while (0);

    return(nLLDRe);
}

/**
 * @brief          this function gets lock status.
 *
 * @param[in]      nPbn        : block number
 * @param[out]     pnLockStat  : lock status
 * @param[out]     nFlag       : O1N_LLD_FLAG_OTP_BLK_STATUS   
 *                               O1N_LLD_FLAG_NORMAL_BLK_STATUS
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark
 *
 */
PUBLIC INT32
O1N_LLD_GetBlockStat(UINT32  nPbn,
                     UINT32 *pnLockStat,
                     UINT32  nFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

            UINT32              nRegVal;

            INT32               nLLDRe     = O1N_LLD_RETURN_SUCCESS;


    do
    {
        switch (nFlag)
        {
        case O1N_LLD_FLAG_OTP_BLK_STATUS:


            /* set DBS */
            O1N_WRITE(pstONDReg->nStartAddr2, 
             ((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DBS);

            /* set DFS & FBA */
            O1N_WRITE(pstONDReg->nStartAddr1, 
                      (((UINT16) nPbn << pstONDCxt->nDDPSelSft) & O1N_LLD_MASK_DFS) |
                       ((UINT16) nPbn & pstONDCxt->nFBAMask));

            /* Write 0 to interrupt register */
            O1N_WRITE(pstONDReg->nInt, O1N_LLD_INT_CLEAR);

            /* Write 'OTP Access' command */
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_OTP_ACCESS);

            /* Wait for INT register low to high transition */
            WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

            /* check the control status register to confirm the lock bit */
            nRegVal = (UINT32) ((O1N_READ(pstONDReg->nCtrlStat) & O1N_LLD_MASK_OTP) >> O1N_LLD_OTP_SHIFT);

            /*
             * Controller Status Register 
             *     | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
             *     |====|====|====|====|====|====|====|====|====|====|====|====|====|====|====|====|
             *     |OnGo|Lock|Load|Prog|Erse| Err| Sus| Rsv|RSTB|OTP |OTP |Prv1|Cur1|Prv2|Cur2| TO |
             *                                                   (l)  (BL)      
             */
            if (nRegVal == 0x01)
            {
                *pnLockStat = O1N_LLD_OTP_OTP_BLK_UNLKED | 
                              O1N_LLD_OTP_1ST_BLK_LOCKED;
            }
            else if (nRegVal == 0x02)
            {
                *pnLockStat = O1N_LLD_OTP_OTP_BLK_LOCKED | 
                              O1N_LLD_OTP_1ST_BLK_UNLKED;
            }
            else if (nRegVal == 0x03)
            {
                *pnLockStat = O1N_LLD_OTP_OTP_BLK_LOCKED | 
                              O1N_LLD_OTP_1ST_BLK_LOCKED;
            }
            else
            {
                *pnLockStat = O1N_LLD_OTP_OTP_BLK_UNLKED | 
                              O1N_LLD_OTP_1ST_BLK_UNLKED;
            }

            break;

        case O1N_LLD_FLAG_NORMAL_BLK_STATUS:

            /* check the control status register to confirm the lock bit */
            nRegVal = (UINT32) ((O1N_READ(pstONDReg->nCtrlStat) & O1N_LLD_MASK_LOCKBIT) >> O1N_LLD_LOCK_SHIFT);

            if (nRegVal == 0)
            {
                *pnLockStat = O1N_LLD_BLK_UNLKED;
            }
            else if (nRegVal == 1)
            {
                *pnLockStat = O1N_LLD_BLK_LOCKED;
            }
            else
            {
                O1N_LLD_PRINT((TEXT("[O1N:ERR]   Irregal case\r\n")));
                nLLDRe = O1N_LLD_RETURN_ERROR;
            }

            break;

        default:

            break;
        }

    } while (0);

    return(nLLDRe);
}

/**
 * @brief          this function resets device.
 *
 * @param[in]      nFlag       : O1N_LLD_FLAG_HOT_RESET  
 *                               O1N_LLD_FLAG_NCORE_RESET
 *
 * @return         O1N_LLD_RETURN_SUCCESS
 * @return         O1N_LLD_RETURN_ERROR
 *
 * @author         JeongWook Moon
 * @version        2.0.0
 * @remark
 *
 */
PUBLIC INT32
O1N_LLD_Reset(UINT32  nFlag)
{
             OneNANDCxt       *pstONDCxt  = &gstONDCxt;
    volatile OneNANDReg       *pstONDReg   = (volatile OneNANDReg *) pstONDCxt->nBaseAddr;

            INT32               nLLDRe     = O1N_LLD_RETURN_SUCCESS;

    do
    {

        /* Wait for INT register low to high transition */
        WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

        if (nFlag == O1N_LLD_FLAG_HOT_RESET)
        {
            /* Write 'Load' command */
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_HOT_RESET);

            WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);

            O1N_WRITE(pstONDReg->nSysConf1, pstONDCxt->nSysConf1);
        }
        else if (nFlag == O1N_LLD_FLAG_NCORE_RESET)
        {
            /* Write 'Load' command */
            O1N_WRITE(pstONDReg->nCmd, O1N_LLD_CMD_RST_NFCORE);

            WAIT_O1N_INT_STAT(pstONDReg, O1N_LLD_INT_READY);
        }
        else
        {
            O1N_LLD_PRINT((TEXT("[O1N:ERR]   Irregal Flag\r\n")));
            nLLDRe = O1N_LLD_RETURN_ERROR;
        }
    } while (0);

    return nLLDRe;
}
