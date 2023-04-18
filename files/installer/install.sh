#!/bin/sh

# fork of https://github.com/dangowrt/owrt-ubi-installer
# GPLv2

. /lib/upgrade/nand.sh

sleep 1

echo
echo OpenWrt UBI installer
echo

INSTALLER_DIR="/installer"
PRELOADER="$INSTALLER_DIR/openwrt-mediatek-mt7622-samknows_whitebox-v9plus-preloader.bin"
FIP="$INSTALLER_DIR/openwrt-mediatek-mt7622-samknows_whitebox-v9plus-bl31-uboot.fip"
RECOVERY="$INSTALLER_DIR/openwrt-mediatek-mt7622-samknows_whitebox-v9plus-initramfs-recovery.itb"
SYSUPGRADE="$INSTALLER_DIR/openwrt-mediatek-mt7622-samknows_whitebox-v9plus-squashfs-sysupgrade.itb"
HAS_ENV=1

if [ ! -s "$PRELOADER" ] || [ ! -s "$FIP" ] || [ ! -s "$RECOVERY" ]; then
	echo "Missing files. Aborting. rebooting in 3 seconds"
	sleep 3
	reboot
	exit 1
fi

install_prepare_ubi() {
	mtddev=$1
	[ -e /sys/class/ubi/ubi0 ] && ubidetach -p $mtddev
	ubiformat -y $mtddev
	sleep 1
	ubiattach -p $mtddev
	sync
	sleep 1
	[ -e /sys/class/ubi/ubi0 ] || exit 1
	devminormajor=$(cat /sys/class/ubi/ubi0/dev)
	oIFS=$IFS
	IFS=':'
	set -- $devminormajor
	devminor=$1
	devmajor=$2
	IFS=$oIFS
	[ -e /dev/ubi0 ] || mknod /dev/ubi0 c $devminor $devmajor
	[ "$HAS_ENV" = "1" ] && ubimkvol /dev/ubi0 -n 0 -s 1MiB -N ubootenv && ubimkvol /dev/ubi0 -n 1 -s 1MiB -N ubootenv2
}

echo "redundantly write bl2 into the first 4 blocks"
for bl2start in 0x0 0x20000 0x40000 0x60000 ; do
	mtd -p $bl2start write $PRELOADER /dev/mtd0
done

echo "write FIP to NAND"
mtd write $FIP /dev/mtd1

echo "read and write factory partition"
dd if=/dev/mtd2 of=/tmp/factory.bin
mtd -e factory write /tmp/factory.bin factory

install_prepare_ubi /dev/mtd3
sync

echo "write recovery ubi volume"
RECOVERY_SIZE=$(cat $RECOVERY | wc -c)
ubimkvol /dev/ubi0 -s $RECOVERY_SIZE -n 2 -N recovery
ubiupdatevol /dev/$(nand_find_volume ubi0 recovery) $RECOVERY

SYSUPGRADE_SIZE=$(cat $SYSUPGRADE | wc -c)
echo "writing ubi A partition"
ubimkvol /dev/ubi0 -s $SYSUPGRADE_SIZE -n 3 -N a_fit
ubiupdatevol /dev/$(nand_find_volume ubi0 a_fit) $SYSUPGRADE

echo "writing ubi B partition"
ubimkvol /dev/ubi0 -s $SYSUPGRADE_SIZE -n 4 -N b_fit
ubiupdatevol /dev/$(nand_find_volume ubi0 b_fit) $SYSUPGRADE

echo "writing a factory.bin backup to ubi"
FACTORY_SIZE=$(cat /tmp/factory.bin | wc -c)
ubimkvol /dev/ubi0 -s $FACTORY_SIZE -n 5 -N factorybackup
ubiupdatevol /dev/$(nand_find_volume ubi0 factorybackup) /tmp/factory.bin

sync
echo "Device is ready! Rebooting in 3 seconds"
sleep 3
reboot -f
