# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=802.1x
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install


include $(INCLUDE_DIR)/package.mk

define Package/802.1x
	SECTION:=802.1x
	CATEGORY:=802.1x
	TITLE:=802.1x
endef

define Package/802.1x/description
	802.1x authentication module.
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
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header/"
	LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"
endef

define Package/802.1x/install
	echo "802.1x module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/rtdot1xd $(1)/usr/bin/
	$(INSTALL_BIN) files/get_dot1x_portControl $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/etc/init.d/dot1x $(1)/etc/init.d/dot1x
	$(INSTALL_DATA) files/etc/rtapd.conf $(1)/etc/
endef

$(eval $(call BuildPackage,802.1x))
