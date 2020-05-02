# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=realtek-ssdk
PKG_VERSION:=1.3.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/realtek-ssdk-1.3.4.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/realtek-ssdk
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=Realtek Unmanaged Switch API
endef

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
	-DSPI_OPERATION \
	-D_LITTLE_ENDIAN

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
else
define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) switch.mk $(PKG_BUILD_DIR)/Makefile
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
endef
define Package/realtek-ssdk/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/librtkssdk.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
endef
else
define Package/realtek-ssdk/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/librtkssdk.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
endef
endif

$(eval $(call BuildPackage,realtek-ssdk))
