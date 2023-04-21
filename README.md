# skwp9 Installer

Installs all bootloaders, production, recovery images to the device.

## Why do I need to use an installer?

The problem lies in the badblock management from the mediatek sdk (bmt).
The mediatek bmt badblock management is a layer which might remappes blocks
of the whole flash.
The idea of the mediatek badblock bmt layer is to treat a SPI-NAND flash
as NOR. It means it can remap badblocks without showing them to the users of the mtd-framework.

However there is already a good known and upstream badblock management in UBI.

## How does it work?

Either boot the installer via tftp or flash it as sysupgrade.
The installer itself is **always** an initramfs-image, even when used with sysupgrade.
It ensures it doesn't have any usage of the flash.

The installer erase and flash all images except the factory partition.
It further creates a copy of the factory partition in the UBI part.

Within the OpenWrt init system it hooks into the pre-init phase to get started.
See: 
* ./files/lib/preinit/00_installer
* ./files/installer/install.sh for further information.

## How to build an installer?

Copy the build artifacts to ./files/installer/

- recovery:

```
openwrt-mediatek-mt7622-samknows_whitebox-v9plus-initramfs-recovery.itb
```

- normal build:
```
openwrt-mediatek-mt7622-samknows_whitebox-v9plus-bl31-uboot.fip
openwrt-mediatek-mt7622-samknows_whitebox-v9plus-preloader.bin
openwrt-mediatek-mt7622-samknows_whitebox-v9plus-squashfs-sysupgrade.itb
```

## Thanks

Thanks to Daniel Golle's [ower-ubi-installer](https://github.com/dangowrt/owrt-ubi-installer).
