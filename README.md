# OpenWrt whitebox 9+ installer

An image to be build as initramfs for the OEM bootloader.
The installer image will flash the basic image.

## How to use?

You need to copy the following files to ./files/installer/

```
(prod) openwrt-mediatek-mt7622-samknows_whitebox-v9plus-bl31-uboot.fip
(prod) openwrt-mediatek-mt7622-samknows_whitebox-v9plus-preloader.bin
(prod) openwrt-mediatek-mt7622-samknows_whitebox-v9plus-squashfs-sysupgrade.itb
(recovery) openwrt-mediatek-mt7622-samknows_whitebox-v9plus-initramfs-recovery.itb
```

Build and take the initramfs image.

## How it works?

All partitions are read/writable.
The ./files/installer/install.sh is executed in the preinit phase.
See the install.sh for more info.
