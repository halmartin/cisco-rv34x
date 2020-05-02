# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk-ms-sdk
PKG_VERSION:=2.1.4.53590
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-src-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/sdk

include $(INCLUDE_DIR)/package.mk

PKG_BIN_SRC=$(TOPDIR)/dl-bin/rtk-ms-sdk-2.1.4.53590.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/rtk-ms-sdk
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=Realtek Managed Switch API
  DEPENDS:=+libpthread
endef

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -Wno-enum-compare -Wno-pointer-sign -Wno-array-bounds -Wno-sizeof-pointer-memaccess" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(TARGET_CONFIGURE_OPTS) \
	SDKDIR=$(PKG_BUILD_DIR) \
	SDK_BUILD=$(PKG_BUILD_DIR)/build

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
endef
else
define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)/lib
	$(TAR) -cC ./src . | $(TAR) -xC $(PKG_BUILD_DIR)
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
endef
else
define Build/Compile
	$(MAKE_FLAGS) $(MAKE) -C $(PKG_BUILD_DIR)/src
	$(MAKE_FLAGS) $(MAKE) -C $(PKG_BUILD_DIR)/system -f $(PKG_BUILD_DIR)/build/Makefile.linux.user.rtcore
	$(MAKE_FLAGS) $(MAKE) -C $(PKG_BUILD_DIR)/switch -f $(PKG_BUILD_DIR)/build/Makefile.linux.user.sw
	$(MAKE_FLAGS) $(MAKE) -C $(PKG_BUILD_DIR)/src/app/diag_v2/src -f $(PKG_BUILD_DIR)/build/Makefile.app.diag all
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Package/rtk-ms-sdk/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/app/diag_v2/bin/diag $(1)/usr/sbin/rtk_ms_switch
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/app/diag_v2/bin/diag_client $(1)/usr/sbin/rtkssdk_diag
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/ms-switch $(1)/etc/init.d/ms-switch
endef
else
define Package/rtk-ms-sdk/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/app/diag_v2/bin/diag $(1)/usr/sbin/rtk_ms_switch
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/app/diag_v2/bin/diag_client $(1)/usr/sbin/rtkssdk_diag
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/ms-switch $(1)/etc/init.d/ms-switch
endef
endif


$(eval $(call BuildPackage,rtk-ms-sdk))
