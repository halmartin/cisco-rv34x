# 2018-01-04: Ganesh Reddy <ganesh.reddy@nxp.com>
# Add c2k_gpio_reset module support.
# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=c2krv340_gpio_reset

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/kmod-c2k_gpio_reset
  CATEGORY:=Freescale Packages
  DEFAULT:=y
  DEPENDS:=
  TITLE:=C2k gpio reset driver
  URL:=http://www.freescale.com/
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
endef

define Package/kmod-c2k_gpio_reset/Description
  The driver is used for GPIO reset
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(LINUX_DIR) \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		modules
	$(MAKE) -C $(LINUX_DIR) \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		INSTALL_MOD_PATH="$(PKG_INSTALL_DIR)" \
		modules_install
endef

define Package/kmod-c2k_gpio_reset/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/lib/modules/$(LINUX_VERSION)/extra/* $(1)/lib/modules/$(LINUX_VERSION)/
	$(CP) ./files/etc $(1)
endef


$(eval $(call BuildPackage,kmod-c2k_gpio_reset))
