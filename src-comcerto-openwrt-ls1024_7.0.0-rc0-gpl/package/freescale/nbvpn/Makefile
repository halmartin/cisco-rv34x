# 
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nbvpn
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/nbvpn-1.0.1.tar.gz
ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define KernelPackage/nbvpn
  CATEGORY:=Freescale Packages
  SECTION:=Freescale Packages
  TITLE:=NBVPN Module Module
  URL:=http://www.freescale.com/
  FILES:= $(PKG_BUILD_DIR)/sbr_cdev.$(LINUX_KMOD_SUFFIX) $(PKG_BUILD_DIR)/nbvpn.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/nbvpn/description
 This package is for sending NetBIOS packets to remote over VPN 
endef

define Package/nbvpn
  CATEGORY:=Freescale Packages
  SECTION:=Freescale Packages
  TITLE:=NBVPN Userspace Module
  URL:=http://www.freescale.com/
endef

define Package/nbvpn/description
 This package is for sending NetBIOS packets to remote over VPN 
endef
ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)
	tar -zxf $(PKG_BIN_SRC) -C $(KERNEL_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/ -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
else
define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -a src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR)/ -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
endif


#EXTRA_CFLAGS += -DDEBUG
ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)
endef

else
define Build/Compile
	$(call Build/Compile/Default, CROSS_COMPILE="$(TARGET_CROSS)" KERNEL_SOURCE="$(LINUX_DIR)" CCOPT="$(TARGET_CFLAGS)" DESTDIR="$(PKG_INSTALL_DIR)")
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/app \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KERNEL_SOURCE="$(LINUX_DIR)" \
		CCOPT="$(TARGET_CFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" all install
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Package/nbvpn/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nbvpn $(1)/usr/sbin/
endef
else
define Package/nbvpn/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nbvpn $(1)/usr/sbin/
endef
endif

$(eval $(call KernelPackage,nbvpn))
$(eval $(call BuildPackage,nbvpn))
