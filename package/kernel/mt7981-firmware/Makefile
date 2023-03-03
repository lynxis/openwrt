include $(TOPDIR)/rules.mk

PKG_NAME:=mt7981-firwmare
PKG_VERSION:=0.1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Alexander Couzens <lynxis@fe80.eu>


include $(INCLUDE_DIR)/package.mk

define Package/mt7981-firmware
	SECTION:=kernel
	CATEGORY:=wip
	TITLE:=mt7981-firmware packages
	DEPENDS:=
endef

define Package/mt7981-firmware/description
	mt7981-firmware stuff
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/mt7981-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	$(INSTALL_DATA) ./files/mt7981_rom_patch.bin ./files/mt7981_wa.bin ./files/mt7981_wm.bin ./files/mt7981_wo.bin $(1)/lib/firmware/mediatek
endef

$(eval $(call BuildPackage,mt7981-firmware))
