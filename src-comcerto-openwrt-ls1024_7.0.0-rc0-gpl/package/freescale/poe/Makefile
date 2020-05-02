# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
#include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=poe
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

PD69200SDK_PKG=pd69200
PD69200_PKG_BUILD_DIR=$(BUILD_DIR)/$(PD69200SDK_PKG)/poe_host_comm_PD69200_workspace

CONFD_PKG=confd-6.5.2
CONFD_PKG_BUILD_DIR=$(BUILD_DIR)/$(CONFD_PKG)

PKG_BUILD_DEPENDS:=confd

include $(INCLUDE_DIR)/package.mk

#PKG_UPGRADE_INFO=$(PKG_BUILD_DIR)/files/poeupgrade.info
PKG_UPGRADE_INFO=./files/poeupgrade.info
include $(PKG_UPGRADE_INFO)

define Package/poe
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=PoE
	DEPENDS:=+confd +pd69200 +libuci
endef

define Package/poe/description
	Power Over Ethernet Module.
endef

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
	-DSPI_OPERATION \
	-D_LITTLE_ENDIAN

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)/
	$(CP) -a files $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include " PD_SDK_DIR="$(PD69200_PKG_BUILD_DIR)" CONFD_LIB="$(CONFD_PKG_BUILD_DIR)/lib/libconfd.a" LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"

endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include " PD_SDK_DIR="$(PD69200_PKG_BUILD_DIR)" CONFD_LIB="$(CONFD_PKG_BUILD_DIR)/lib/libconfd.so" LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"

endef
	#v2.1.1
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/22021119_0816_003.s19 $(1)/usr/bin/postupgrade
	#v1.8.7
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/22018701_0801_021.s19 $(1)/usr/bin/postupgrade
	#v1.7.2
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/22017203_0800_089.s19 $(1)/usr/bin/postupgrade
	#v1.6.1
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/22016103_0800_073.s19 $(1)/usr/bin/postupgrade
define Package/poe/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/poe.init $(1)/etc/init.d/poeinit
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/bin/poe_cmd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/bin/poemon $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/chkpoeconf.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/poefwupgrade.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/poevop_chk.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/bin/poeupgrade $(1)/usr/bin/
ifdef POE_FW_VERSION
	echo "#!/bin/sh" > $(POE_SCRIPT_FIL)
	$(POE_UPGRADE_CMD)
	$(INSTALL_BIN) $(POE_SCRIPT_FIL) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/$(POE_FW_INSTALL_PATH)
	$(INSTALL_BIN) $(POE_FIRMWARE_FILE_PATH) $(1)/$(POE_FW_INSTALL_PATH)
endif
endef

$(eval $(call BuildPackage,poe))
