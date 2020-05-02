# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pd69200
PKG_VERSION:=2_0_18
PKG_SOURCE_URL:="http://microsemi.com/"
PKG_SOURCE:=poe_host_comm_PD69200_workspace_ver_2_0_18.zip
PKG_MD5SUM:=17323086e811d2468b9e2cbcc39f44cb
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/pd69200
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_WORKSPACE_DIR:=$(PKG_BUILD_DIR)/poe_host_comm_PD69200_workspace

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/pd69200.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/pd69200
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Serial Communication Protocol
endef

define Package/pd69200/description
	Microsemi SDK for PD69200 Serial Communication Protocol.
endef
ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
	echo "prepare done..."
endef
else
define Build/Prepare
	unzip $(DL_DIR)/$(PKG_SOURCE) -d $(PKG_BUILD_DIR)
	echo "prepare done..."
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
endef
else
define Build/Compile
	$(MAKE) -C $(PKG_WORKSPACE_DIR)/poe_host_comm_api  $(TARGET_CONFIGURE_OPTS)
	$(MAKE) -C $(PKG_WORKSPACE_DIR)/mscc_arcitecture  $(TARGET_CONFIGURE_OPTS)
endef
endif

define Package/pd69200/install
	echo "install"
endef
#	$(INSTALL_DIR) $(1)/lib
#	$(INSTALL_BIN) $(PKG_WORKSPACE_DIR)/poe_host_comm_api/lib/poe_host_comm_api_lib.a $(1)/lib/
#	$(INSTALL_BIN) $(PKG_WORKSPACE_DIR)/mscc_arcitecture/lib/mscc_arcitecture_lib.a $(1)/lib/

$(eval $(call BuildPackage,pd69200))
