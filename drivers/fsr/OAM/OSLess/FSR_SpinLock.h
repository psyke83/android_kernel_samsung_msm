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
 * @file      FSR_SpinLock.h
 * @brief
 * @author
 * @date
 * @remark
 * REVISION HISTORY
 *
 */
#ifndef _FSR_SPINLOCK_H
#define _FSR_SPINLOCK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define     FSR_SMEM_SPINLOCK_BASEADDR          (0x07FFFFF0)

#define     FSR_SMEM_SPINLOCK_BASEADDR          (SCL_SHARED_RAM_BASE+SCL_SHARED_RAM_SIZE-0x10)

#define     FSR_SMEM_SPINLOCK_BML               (0)
#define     FSR_SMEM_SPINLOCK_BOOT              (1)
#define     FSR_SMEM_SPINLOCK_FS_REBUILD        (2)
/* do not use 0x1FFFFFC
   because FS_REBUILD_FLAG_POS use the 0x1FFFFFC
 */

void smem_flash_spin_lock(UINT32 nIdx, UINT32 nBaseAddr);
void smem_flash_spin_unlock(UINT32 nIdx, UINT32 nBaseAddr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FSR_SPINLOCK_H */

