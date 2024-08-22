include $(TOPDIR)/rules.mk

PKG_NAME:=uqmid
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/lynxis/uqmi.git
PKG_SOURCE_DATE:=2024-10-10
PKG_SOURCE_VERSION:=3b6324f2ab15d65060e7070bd1ded3f4e1ece9ab
PKG_MAINTAINER:=Alexander Couzens <lynxis@fe80.eu>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_FLAGS:=nonshared
PKG_BUILD_FLAGS:=gc-sections
PKG_BUILD_DEPENDS:=libtalloc

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/uqmid
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +libblobmsg-json +kmod-usb-net +kmod-usb-net-qmi-wwan +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/uqmid/description
  uqmi is a command line tool for controlling mobile broadband modems using
  the QMI-protocol.
endef

define Package/uqmid
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +libubus +libblobmsg-json +libtalloc +kmod-usb-net +kmod-usb-net-qmi-wwan +wwan
  TITLE:=Control daemon for mobile broadband modems
endef

define Package/uqmid/description
  uqmid is a daemon for controlling mobile broadband modems using
  the QMI-protocol.
endef


TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-Wno-error=maybe-uninitialized

CMAKE_OPTIONS += \
	-DDEBUG=1 \
	-DBUILD_UQMID=ON

define Package/uqmid/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmid/uqmid $(1)/sbin/
	$(INSTALL_DIR) $(1)/lib/netifd/proto/ $(1)/etc/init.d/ $(1)/etc/hotplug.d/usbmisc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmid/uqmid.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/etc/init.d/uqmid $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmid/uqmid.hotplug.sh $(1)/etc/hotplug.d/usbmisc
endef

$(eval $(call BuildPackage,uqmid))

