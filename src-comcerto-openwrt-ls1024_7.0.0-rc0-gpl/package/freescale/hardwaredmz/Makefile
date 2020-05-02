#
#/**************************************************************************
# *  * Copyright 2010-2015, Freescale Semiconductor, Inc. All rights reserved.
# ***************************************************************************/
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hardwaredmz
PKG_VERSION:=1.00
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/hardwaredmz
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=HardwareDMZ
  URL:=http://www.freescale.com/
  TITLE:=Freescale hardware configuration
endef

define Package/hardwaredmz/description
   Freescale hardwaredmz configuration files.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/hardwaredmz/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/hardwaredmz.init $(1)/etc/init.d/hardwaredmz
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/hardwaredmz.hotplug $(1)/etc/hotplug.d/iface/94-hardware-dmz
endef

$(eval $(call BuildPackage,hardwaredmz))
