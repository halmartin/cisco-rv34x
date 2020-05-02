# 
# Copyright (C) 2015 Freescale
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.0 2015/04/14 16:00:00 alex.winter Exp $


include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=udp-bridge
PKG_VERSION:=1.00.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/kmod-udp-bridge
  CATEGORY:=Freescale Packages
  DEPENDS:=
  DEFAULT:=n
  TITLE:=UDP bridge kernel module
  URL:=http://www.freescale.com/
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
endef

define Package/kmod-udp-bridge/Description
  The UDP bridge module for redirection UDP streams between interfaces
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

define Package/kmod-udp-bridge/install
	mkdir -p $(1)/lib/modules/$(LINUX_VERSION)
	$(CP) -a $(PKG_INSTALL_DIR)/lib/modules/$(LINUX_VERSION)/extra/* \
		$(1)/lib/modules/$(LINUX_VERSION)/
endef


$(eval $(call BuildPackage,kmod-udp-bridge))
