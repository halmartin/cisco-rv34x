#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=confd_relay
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/confd_relay
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:= +confd
  TITLE:=confd useauth (confd_relay).
endef

define Package/confd_relay/description
        This binary is used to relay authentications from external apps tp confd
endef

define Build/Prepare
        $(call Build/Prepare/Default)
		$(CP) -fr src/* $(PKG_BUILD_DIR)
		find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef


define Package/confd_relay/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/userauth $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/notifier $(1)/usr/bin/
endef

$(eval $(call BuildPackage,confd_relay))
