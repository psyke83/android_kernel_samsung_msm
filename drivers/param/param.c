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
#include <linux/device.h>

#include "param.h"
#include <samsung_flash.h>

#if defined(CONFIG_MACH_EUROPA) || defined(CONFIG_MACH_CALLISTO)
#define PARAM_nID				FSR_PARTID_BML9
#else
#define PARAM_nID				FSR_PARTID_BML8
#endif

#define NAND_PAGE_PER_UNIT		64
#define NAND_SECTOR_PER_PAGE	8
#define NAND_PAGE_SIZE 0x1000 

// must be same as bootable/bootloader/lk/app/aboot/common.h
/* PARAM STRUCTURE */
typedef struct _param {
	int booting_now; // 1:boot   0:enter service
	int fota_mode; // 1:recovery mode   0:boot
	int status; // delta update ex) 0x001: KERNEL_UPDATE_SUCCESS , 0x011: KERNEL_UPDATE_SUCCESS + MODEM_UPDATE_SUCCESS

	int gota_mode; // 1: gota, 0: nomal
	int recovery_command;

	char efs_info[32];
	char keystr[32];	// Å° ½ºÆ®¸µ À¯Ãâ ¹æÁö.

#if 1 //defined(CONFIG_MACH_ROOKIE) || defined(CONFIG_MACH_ESCAPE) || defined(CONFIG_MACH_GIO)
	int ram_dump_level;
	int ram_dump_level_init;
#endif
	unsigned int first_boot_done; //rooting information
	unsigned int custom_download_cnt;
	char current_binary[30];

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
	printk("PARAM booting_now : %d\n",efs.booting_now);
	printk("PARAM fota_mode   : %d\n",efs.fota_mode);
	printk("PARAM efs_info	  : %s\n",efs.efs_info);

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

static int param_keystr_read_proc_debug(char *page, char **start, off_t offset, int count, int *eof, void *data)
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
	printk("PARAM::booting_now : %d\n",efs.booting_now);
	printk("PARAM::fota_mode   : %d\n",efs.fota_mode);
	printk("PARAM::efs_info	  : %s\n",efs.efs_info);
	printk("PARAM::keystr	  : %s\n",efs.keystr);

	return sprintf(page, "%s\n", efs.keystr);
}
 
static int param_keystr_write_proc_debug(struct file *file, const char *buffer,
		                            unsigned long count, void *data)
{
	char *buf;
	int err;
	unsigned int nByteRet;
	PARAM efs;
	FSRChangePA stChangePA;

	if (count < 1)
		return -EINVAL;

	if(count > sizeof(efs.keystr))
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
	memset(efs.keystr, 0x0, sizeof(efs.keystr));
	memcpy(efs.keystr, buf, (int)count);
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

extern int (*set_recovery_mode)(void);

int _set_recovery_mode(void)
{
	int err;
	unsigned int nByteRet;
	PARAM param;
	FSRChangePA stChangePA;

    printk("_set_recovery_mode++");

	memset(mBuf, 0xff, NAND_PAGE_SIZE);

	err = samsung_bml_read(get_param_start_unit() * NAND_PAGE_PER_UNIT * NAND_SECTOR_PER_PAGE, NAND_SECTOR_PER_PAGE, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML READ FAIL!\n");
		return err;
	}

	memcpy(&param, mBuf, sizeof(PARAM));
	// copy user data to efs
	param.booting_now = RECOVERY_ENTER_MODE;
	memcpy(mBuf,&param,sizeof(PARAM));

	stChangePA.nPartID  = PARAM_nID;
	stChangePA.nNewAttr = FSR_BML_PI_ATTR_RW;
	if (FSR_BML_IOCtl(0, FSR_BML_IOCTL_CHANGE_PART_ATTR , (UINT8 *) &stChangePA, sizeof(stChangePA), NULL, 0, &nByteRet) != FSR_BML_SUCCESS) {
		return FS_DEVICE_FAIL;
	}

	err = samsung_bml_erase(get_param_start_unit(), 1);
	if(err) {
		printk("PARAMERTER BML ERASE FAIL!\n");
		return err;
	}

	// write first page to param block
	err = samsung_bml_write(get_param_start_unit() * NAND_PAGE_PER_UNIT, 1, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML WRITE FAIL!\n");
		return err;
	}

	stChangePA.nNewAttr = FSR_BML_PI_ATTR_RO;
	if (FSR_BML_IOCtl(0, FSR_BML_IOCTL_CHANGE_PART_ATTR , (UINT8 *) &stChangePA, sizeof(stChangePA), NULL, 0, &nByteRet) != FSR_BML_SUCCESS) {
		return FS_DEVICE_FAIL;
	}

	return 0;

}

extern int (*set_recovery_mode_done)(void);

int _set_recovery_mode_done(void)
{
	int err;
	unsigned int nByteRet;
	PARAM param;
	FSRChangePA stChangePA;

    printk("_set_recovery_mode_done++");

	memset(mBuf, 0xff, NAND_PAGE_SIZE);

	err = samsung_bml_read(get_param_start_unit() * NAND_PAGE_PER_UNIT * NAND_SECTOR_PER_PAGE, NAND_SECTOR_PER_PAGE, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML READ FAIL!\n");
		return err;
	}

	memcpy(&param, mBuf, sizeof(PARAM));
	// copy user data to efs
	param.booting_now = RECOVERY_END_MODE;
	memcpy(mBuf,&param,sizeof(PARAM));

	stChangePA.nPartID  = PARAM_nID;
	stChangePA.nNewAttr = FSR_BML_PI_ATTR_RW;
	if (FSR_BML_IOCtl(0, FSR_BML_IOCTL_CHANGE_PART_ATTR , (UINT8 *) &stChangePA, sizeof(stChangePA), NULL, 0, &nByteRet) != FSR_BML_SUCCESS) {
		return FS_DEVICE_FAIL;
	}

	err = samsung_bml_erase(get_param_start_unit(), 1);
	if(err) {
		printk("PARAMERTER BML ERASE FAIL!\n");
		return err;
	}

	// write first page to param block
	err = samsung_bml_write(get_param_start_unit() * NAND_PAGE_PER_UNIT, 1, mBuf, NULL);
	if(err) {
		printk("PARAMERTER BML WRITE FAIL!\n");
		return err;
	}

	stChangePA.nNewAttr = FSR_BML_PI_ATTR_RO;
	if (FSR_BML_IOCtl(0, FSR_BML_IOCTL_CHANGE_PART_ATTR , (UINT8 *) &stChangePA, sizeof(stChangePA), NULL, 0, &nByteRet) != FSR_BML_SUCCESS) {
		return FS_DEVICE_FAIL;
	}

    printk("_set_recovery_mode_done--");

	return 0;

}

static int __init param_init(void)
{
 	struct proc_dir_entry *ent, *ent2;
		
	ent = create_proc_entry("efs_info", S_IFREG | S_IWUSR | S_IRUGO, fsr_proc_dir);
	ent->read_proc = param_read_proc_debug;
	ent->write_proc = param_write_proc_debug;

	ent2 = create_proc_entry("keystr", S_IFREG | S_IWUSR | S_IRUGO, fsr_proc_dir);
	ent2->read_proc = param_keystr_read_proc_debug;
	ent2->write_proc = param_keystr_write_proc_debug;
	set_recovery_mode = _set_recovery_mode;
	set_recovery_mode_done = _set_recovery_mode_done;

	return 0;
}

static void __exit param_exit(void)
{
	remove_proc_entry("efs_info", fsr_proc_dir);
	remove_proc_entry("keystr", fsr_proc_dir);
}

module_init(param_init);
module_exit(param_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Samsung Electronics");
MODULE_DESCRIPTION("Samsung Param Operation");
