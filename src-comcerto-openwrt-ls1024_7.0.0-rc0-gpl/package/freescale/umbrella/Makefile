#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=Umbrella
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/Umbrella
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=Umbrella
	DEPENDS:=+libuci
endef

define Package/system/description
	This is the cisco's cloud security service for DNS queries which use DNSCrypt at client side and OpenDNS at server side.
endef


#MAKE_FLAGS += \

define Build/Prepare
#	$(call Build/Prepare/Default)
#	$(CP) -a src $(PKG_BUILD_DIR)/
#	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

define Build/Compile
#	$(MAKE) -C $(PKG_BUILD_DIR) \
#	$(TARGET_CONFIGURE_OPTS) \
#	CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/Umbrella/install
	echo "Umbrella module install !!"
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) ./files/umbrella.init $(1)/etc/init.d/umbrella
	$(INSTALL_BIN) ./files/umbrella.config $(1)/etc/config/umbrella
	$(INSTALL_BIN) ./files/rpc/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,Umbrella))
