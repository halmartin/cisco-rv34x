# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vpnresourcemgmt
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/vpnresourcemgmt
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=VPN Resource Management
	DEPENDS:=+strongswan +pptpd
endef

define Package/vpnresourcemgmt/description
	vpnresourcemgmt is for managing number of connections per VPN module.
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/vpnresourcemgmt/install
	echo "vpnresourcemgmt module install !!"
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnresourcemgmt.init $(1)/etc/init.d/vpnresourcemgmt
endef

$(eval $(call BuildPackage,vpnresourcemgmt))
