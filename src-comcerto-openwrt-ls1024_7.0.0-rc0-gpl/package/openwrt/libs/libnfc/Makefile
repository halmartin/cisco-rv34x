#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnfc
PKG_VERSION:=2.1
PKG_RELEASE:=1
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/NXPNFCLinux/linux_libnfc-nci
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=LGPL-2.1
PKG_MAINTAINER:=Sebastian Wendel <packages@sourceindex.de>

include $(INCLUDE_DIR)/package.mk

#DEPENDS:=+libusb-compat +pcscd +ccid +libstdcpp Removed the pcsd
#and ccid packages from  dependencies as they are not required
#now.
define Package/libnfc/Default
	TITLE:=A open source library for Near Field Communication (NFC)
	URL:=http://nfc-tools.org/
endef

define Package/libnfc
	$(call Package/libnfc/Default)
	SECTION:=libs
	CATEGORY:=Libraries
	DEFAULT:=n
	DEPENDS:=+libusb-compat  +libstdcpp
endef

define Package/libnfc/description
	libnfc is the first libre, platform-independent, low level NFC SDK and Programmers API

	* manipulate Jewel Topaz tags using libnfc
	* manipulate MIFARE Classic and Ultralight tags using libnfc

endef


TARGET_CFLAGS= -g -O2
CONFIGURE_ARGS+=--without-readline --enable-pn7150

define Build/Compile
	        $(MAKE) -C $(PKG_BUILD_DIR) \
	                DESTDIR="$(PKG_INSTALL_DIR)" \
	                all install
endef
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnfc* $(1)/usr/lib/
endef

define Package/libnfc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/conf
	$(INSTALL_DIR) $(1)/etc/conf/PN7150
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/conf/PN7150/libnfc-nxp.conf $(1)/etc/
	$(LN) /etc/libnfc-nxp.conf $(1)/etc/conf
	$(LN) /etc/libnfc-nxp.conf  $(1)/etc/conf/PN7150/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnfc* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/.libs/nfcDemoApp $(1)/usr/sbin/
endef


$(eval $(call BuildPackage,libnfc))
