How to build platform

1. Get android open source.
    : version info - Android gingerbread 2.3.6
    ( Download site : http://source.android.com )

2. Overwrite modules that you want to build.

3. \external\ip : Write "ip \" into "build\core\user_tags.mk" so that add this module.
  
4. Add the following lines at the end of build/target/board/generic/BoardConfig.mk

BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true
BT_ALT_STACK := true
BRCM_BT_USE_BTL_IF := true
BRCM_BTL_INCLUDE_A2DP := true

5. ./build.sh user