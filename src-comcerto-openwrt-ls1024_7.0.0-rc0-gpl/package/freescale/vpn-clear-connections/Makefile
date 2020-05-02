# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vpn-clear-connections
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/vpn-clear-connections
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=Utility to clear connections given its SRC subnect and DST subnet
	DEPENDS:=+libnetfilter-conntrack
endef

define Package/vpn-clear-connections/description
	This package provides and utility using which we can clear connections.
	The filters like SRC subnet of the connections and DST subnet of the connection are mandatory.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src/* $(PKG_BUILD_DIR)
endef

define Package/vpn-clear-connections/install
	echo "vpn-clear-connections module install !!"
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vpn_clear_connections $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,vpn-clear-connections))
