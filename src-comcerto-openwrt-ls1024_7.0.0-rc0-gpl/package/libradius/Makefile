#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libradius
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PKG_SOURCE:=pam_radius-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://ftp.freeradius.org/pub/freeradius/
PKG_MD5SUM:=80960fdc9b720677dbb51d17311664a0
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_BUILD_DIR:=$(BUILD_DIR)/pam_radius-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libradius
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpam
  TITLE:=free implementation of the following DCE-RFC from Sunsoft.
  URL:=http://freeradius.org
endef

define Package/libradius/description
        Linux-PAM is a free implementation of the following DCE-RFC from Sunsoft. 

endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); autoreconf -v -f --install || exit 1)
	$(call Build/Configure/Default
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/security
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/lib/security/
endef

define Package/libradius/install
	$(INSTALL_DIR) $(1)/lib 
	$(INSTALL_DIR) $(1)/lib/security 
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/lib/security/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/radius.conf $(1)/etc/config/radius
	$(INSTALL_DIR) $(1)/etc/raddb/
	 $(INSTALL_DATA) ./files/server $(1)/etc/raddb/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/radius.init $(1)/etc/init.d/radius
endef

$(eval $(call BuildPackage,libradius))
