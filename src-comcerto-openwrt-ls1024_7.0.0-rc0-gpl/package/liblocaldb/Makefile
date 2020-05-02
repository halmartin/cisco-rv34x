#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=liblocaldb
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/pam_localdb-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/liblocaldb
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpam +openldap-server
  TITLE:=pam localdb.
endef

define Package/liblocaldb/description
        This library is for local authorisation and authentication. 

endef

define Build/Prepare
        $(call Build/Prepare/Default)
		$(CP) -fr src/* $(PKG_BUILD_DIR)
		find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/security
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/lib/security/
endef

define Package/liblocaldb/install
	$(INSTALL_DIR) $(1)/lib 
	$(INSTALL_DIR) $(1)/lib/security 
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/aaa $(1)/etc/init.d/aaa
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/lib/security/
	$(CP) $(PKG_BUILD_DIR)/weblogin $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liblocaldb))
