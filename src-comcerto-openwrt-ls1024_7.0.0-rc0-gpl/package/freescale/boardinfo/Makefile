#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=boardinfo
PKG_VERSION:=1.0
PKG_RELEASE:=

include $(TOPDIR)/rules.mk

PKG_MAINTAINER:=

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/boardinfo
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=boardinfo
	DEPENDS:=+firmware
endef

define Package/boardinfo/description
	RV340 boardinfo
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
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  -L$(TARGET_LDFLAGS) $(TARGET_CFLAGS_EXTRA)"
endef

define Build/Install
	$(Build/Install/boardinfo)
endef


define Package/boardinfo/install
	echo "boardinfo install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/get_boardinfo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,boardinfo))
