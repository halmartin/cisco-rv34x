# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bcm-jumbo-frame
PKG_VERSION:=1.0
PKG_RELEASE:=0

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
PKG_BIN_SRC=$(TOPDIR)/package/freescale/$(PKG_NAME)/src
#PKG_BIN_FILES=$(TOPDIR)/package/freescale/$(PKG_NAME)/files

include $(INCLUDE_DIR)/package.mk

define KernelPackage/bcm-jumbo-frame
	CATEGORY:=Freescale Packages
	SECTION:=Freescale Packages
	TITLE:=BCM Kernel Module
	FILES:= $(PKG_BUILD_DIR)/jumbo_frame.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/bcm-jumbo-frame/description
	BCM module to set and get register.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(PKG_BIN_SRC)/* $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR) -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

define Build/Compile
	$(call Build/Compile/Default, CROSS_COMPILE="$(TARGET_CROSS)" KERNEL_SOURCE="$(LINUX_DIR)" CCOPT="$(TARGET_CFLAGS)" DESTDIR="$(PKG_INSTALL_DIR)")
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNEL_SOURCE="$(LINUX_DIR)" \
	CCOPT="$(TARGET_CFLAGS)" \
	DESTDIR="$(PKG_INSTALL_DIR)" all 
endef

define Package/bcm-jumbo-frame/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/lib/modules/$(LINUX_VERSION)/extra/* $(1)/lib/modules/$(LINUX_VERSION)/
	$(CP) ./files/etc $(1)
endef

$(eval $(call KernelPackage,bcm-jumbo-frame))
