HOW TO BUILD KERNEL 2.6.35 FOR GT-S5670

1. How to Build
	- get Toolchain
	Visit http://www.codesourcery.com/, download and install Sourcery G++ Lite 2009q3-68 toolchain for ARM EABI.
	Extract kernel source and move into the top directory.
	$ toolchain\arm-eabi-4.4.3
	$ cd kernel/
	$ ./make_kernel_GT-S5670.sh
	
2. Output files
	- Kernel : kernel/arch/arm/boot/zImage
	
3. How to make .tar binary for downloading into target.
	- change current directory to kernel/arch/arm/boot
	- type following command
	$ tar cvf GT-S5670_Kernel_Gingerbread.tar zImage
