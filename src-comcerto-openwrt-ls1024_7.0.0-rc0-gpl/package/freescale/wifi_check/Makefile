# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
#include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wifi_check
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/wifi_check-fsl.1.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/wifi_check
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	DEPENDS:=+uci
	TITLE:=wifi_check
endef

define Package/wifi_check/description
	standby mode and normal mode switch daemon.
endef

TARGET_LDFLAGS += "-luci"

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
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
	$(CP) -a src $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
endef
else
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include  -I$(CISCOSB_YANG_HEADERS_PATH)/header" \
	LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"
endef
endif

define Package/wifi_check/install
	echo "wifi_check module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wifi_check $(1)/usr/bin/
endef

$(eval $(call BuildPackage,wifi_check))
