#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <asm/unistd.h>
#include <asm/errno.h>
#include <asm/uaccess.h>

#include "samsung_flash.h"

#define PARAM_nID				FSR_PARTID_BML9
#define NAND_PAGE_PER_UNIT		64
#define NAND_SECTOR_PER_PAGE	8

// must be same as bootable/bootloader/lk/app/aboot/common.h
typedef struct _param {
	int booting_now;    
	int a;  
	int b;
	int c;
	int d;
	char efs_info[32];
	char str2[32];
} PARAM;

FSRPartI pstPartI;
int param_n1stVun;
char mBuf[NAND_PAGE_SIZE];
extern struct proc_dir_entry *fsr_proc_dir;

static int get_param_start_unit(void)
{
	int cnt;

	if(param_n1stVun == 0) {
		samsung_get_full_bmlparti(&pstPartI);

		for(cnt = 0; cnt < pstPartI.nNumOfPartEntry; cnt++) 
			if(pstPartI.stPEntry[cnt].nID == PARAM_nID) 
				break;

		param_n1stVun = pstPartI.stPEntry[cnt].n1stVun;
	}

	return param_n1stVun;
}

static int param_read_proc_debug(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	int err;
	PARAM efs;

	*eof = 1;
	memset(mBuf, 0xff, NAND_PAGE_SIZE);

	// read first page from param block
	err = samsung_bml_read(get_param_start_unit() * NAND_PAGE_PER_UNIT * NAND_SECTOR_PER_PAGE, NAND_SECTOR_PER_PAGE, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML READ FAIL!\n");
		return err;
	}

	memcpy(&efs, mBuf, sizeof(PARAM));

	return sprintf(page, "%s\n", efs.efs_info);
}
 
static int param_write_proc_debug(struct file *file, const char *buffer,
		                            unsigned long count, void *data)
{
	char *buf;
	int err;
	unsigned int nByteRet;
	PARAM efs;
	FSRChangePA stChangePA;

	if (count < 1)
		return -EINVAL;

	if(count > sizeof(efs.efs_info))
		return -EFAULT;

	buf = kmalloc(count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}

	memset(mBuf, 0xff, NAND_PAGE_SIZE);

	err = samsung_bml_read(get_param_start_unit() * NAND_PAGE_PER_UNIT * NAND_SECTOR_PER_PAGE, NAND_SECTOR_PER_PAGE, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML READ FAIL!\n");
		return err;
	}

	memcpy(&efs, mBuf, sizeof(PARAM));
	// copy user data to efs
	memset(efs.efs_info, 0x0, sizeof(efs.efs_info));
	memcpy(efs.efs_info, buf, (int)count);
	memcpy(mBuf, &efs, sizeof(PARAM));

	stChangePA.nPartID  = PARAM_nID;
	stChangePA.nNewAttr = FSR_BML_PI_ATTR_RW;
	if (FSR_BML_IOCtl(0, FSR_BML_IOCTL_CHANGE_PART_ATTR , (UINT8 *) &stChangePA, sizeof(stChangePA), NULL, 0, &nByteRet) != FSR_BML_SUCCESS) {
		return FS_DEVICE_FAIL;
	}

	err = samsung_bml_erase(get_param_start_unit(), 1);
	if(err) {
		printk("PARAMERTER BML ERASE FAIL!\n");
		kfree(buf);
		return err;
	}

	// read first page from param block
	err = samsung_bml_write(get_param_start_unit() * NAND_PAGE_PER_UNIT, 1, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML WRITE FAIL!\n");
		kfree(buf);
		return err;
	}

	stChangePA.nNewAttr = FSR_BML_PI_ATTR_RO;
	if (FSR_BML_IOCtl(0, FSR_BML_IOCTL_CHANGE_PART_ATTR , (UINT8 *) &stChangePA, sizeof(stChangePA), NULL, 0, &nByteRet) != FSR_BML_SUCCESS) {
		return FS_DEVICE_FAIL;
	}

	kfree(buf);
	return count;
}

int param_init(void)
{
#if 0
	struct proc_dir_entry *ent;

	ent = create_proc_entry("efs_info", S_IFREG | S_IWUSR | S_IRUGO, fsr_proc_dir);
	ent->read_proc = param_read_proc_debug;
	ent->write_proc = param_write_proc_debug;
#endif

	return 0;
}

