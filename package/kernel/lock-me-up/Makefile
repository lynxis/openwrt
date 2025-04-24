#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lock-me-up
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/lock-me-up
  SUBMENU:=Other modules
  TITLE:=Lock Me Up - lock a thread in a while true look
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/lock-me-up.ko
  KCONFIG:=
endef

define KernelPackage/lock-me-up/description
  Kernel module to lock the kernel
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,lock-me-up))
