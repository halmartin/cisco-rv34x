# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk-switch
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/rtk-switch-1.0.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1

else
	FILE_EXISTS = 0
endif

define Package/rtk-switch
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  DEPENDS:=+realtek-ssdk
  TITLE:=Realtek Switch
endef

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
	-DSPI_OPERATION \
	-D_LITTLE_ENDIAN

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="-I $(TOP_DIR)/$(PKG_BUILD_DIR)/../realtek-ssdk-1.3.4/ $(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" 

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)
endef

else
define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) src/main.c $(PKG_BUILD_DIR)/
	$(CP) src/linkmon_main.c $(PKG_BUILD_DIR)/
	$(CP) src/Makefile $(PKG_BUILD_DIR)/
endef

endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
endef
define Package/rtk-switch/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtkssdk $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/swlinkmon $(1)/usr/sbin/
	$(CP) ./files/etc $(1)
endef

else
define Package/rtk-switch/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtkssdk $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/swlinkmon $(1)/usr/sbin/
	$(CP) ./files/etc $(1)
endef

endif

$(eval $(call BuildPackage,rtk-switch))
