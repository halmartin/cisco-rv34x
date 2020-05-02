#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libldap
PKG_VERSION:=186
PKG_RELEASE:=1

PKG_SOURCE:=pam_ldap.tgz
PKG_SOURCE_URL:=http://www.padl.com/download/
PKG_MD5SUM:=58c8689921c5c4578363438acd8503c2
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_BUILD_DIR:=$(BUILD_DIR)/pam_ldap/pam_ldap-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libldap
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+openldap-server libpam
  TITLE:=free implementation of the pamldap.
  URL:=http://padl.com
endef

define Package/libldap/description
        pamldap is a free implementation for ldap authentication using pam. 

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

define Package/libldap/install
	$(INSTALL_DIR) $(1)/lib 
	$(INSTALL_DIR) $(1)/lib/security 
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/lib/security/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/ldap.config $(1)/etc/config/ldap
	$(INSTALL_DATA) ./files/ldap.conf $(1)/etc/ldap.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ldap.init $(1)/etc/init.d/ldapclient
endef

$(eval $(call BuildPackage,libldap))

