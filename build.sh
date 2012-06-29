#!/bin/bash

################################################################################
#
# Build Script
#
# Copyright Samsung Electronics(C), 2010
#
################################################################################


################################################################################
# Useage
#   : ./build.sh [mod]
#
# Example
#   : ./build.sh eng
################################################################################
if	[ "" = "$1" ]
then
	echo --------------------------------------------------------------------------------
	echo - Useage
	echo -   : ./build.sh [mod]
	echo -
	echo - Example
	echo -   : ./build.sh eng
	echo --------------------------------------------------------------------------------
	exit
fi
export CPU_NUMBER=16
export TARGET_BUILD_VARIANT=$1



################################################################################
# Common Path
################################################################################
TOP_DIR=$PWD

# set root path
export PLATFORMPATH="$TOP_DIR"

cd $PLATFORMPATH 
make update-api
make -j$CPU_NUMBER PRODUCT-generic-$TARGET_BUILD_VARIANT
