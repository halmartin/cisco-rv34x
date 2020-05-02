# 
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=firewall_uci
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

IPTABLES_PATH:=$(KERNEL_BUILD_DIR)/iptables-*/

include $(INCLUDE_DIR)/package.mk

define Package/firewall_uci
  CATEGORY:=Freescale Packages
  SECTION:=Freescale Packages
  TITLE:=UCI based iptables firewall
  URL:=http://www.freescale.com/
  DEPENDS:=+libuci +libubus +libxtables +iptables +ip6tables
endef

define Package/firewall_uci/description
 This package is for UCI based iptables firewall
endef

#MAKE_FLAGS += \
	UCIMAPLIB="$(STAGING_DIR)/usr/lib/libucimap.a $(STAGING_DIR)/usr/lib/libip4tc.so $(STAGING_DIR)/usr/lib/libxtables.so $(IPTABLES_PATH)extensions/libiptext.so $(IPTABLES_PATH)extensions/libiptext4.so"

#MAKE_FLAGS += \
	UCIMAPLIB="$(STAGING_DIR)/usr/lib/libucimap.a $(STAGING_DIR)/usr/lib/libip4tc.so $(STAGING_DIR)/usr/lib/libxtables.so $(STAGING_DIR)/usr/lib/libiptext.so $(STAGING_DIR)/usr/lib/libiptext4.so"

MAKE_FLAGS += \
	UCIMAPLIB="$(STAGING_DIR)/usr/lib/libucimap.a $(STAGING_DIR)/usr/lib/libip4tc.so $(STAGING_DIR)/usr/lib/libxtables.so"

#MAKE_FLAGS += \
	UCIMAPLIB6="$(STAGING_DIR)/usr/lib/libucimap.a $(STAGING_DIR)/usr/lib/libip6tc.so $(STAGING_DIR)/usr/lib/libxtables.so $(IPTABLES_PATH)extensions/libiptext6.so $(IPTABLES_PATH)extensions/libiptext.so"

MAKE_FLAGS += \
	UCIMAPLIB6="$(STAGING_DIR)/usr/lib/libucimap.a $(STAGING_DIR)/usr/lib/libip6tc.so $(STAGING_DIR)/usr/lib/libxtables.so"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -a src/* $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/include/iptables
	$(CP) $(IPTABLES_PATH)include/iptables.h $(PKG_BUILD_DIR)/include/
	$(CP) $(IPTABLES_PATH)include/ip6tables.h $(PKG_BUILD_DIR)/include/
	$(CP) $(IPTABLES_PATH)/iptables/iptables-multi.h $(PKG_BUILD_DIR)/include/
	$(CP) $(IPTABLES_PATH)/iptables/ip6tables-multi.h $(PKG_BUILD_DIR)/include/
	$(CP) $(IPTABLES_PATH)include/iptables/internal.h $(PKG_BUILD_DIR)/include/iptables/
	$(CP) $(IPTABLES_PATH)/iptables/iptables.c $(PKG_BUILD_DIR)/
	$(CP) $(IPTABLES_PATH)/iptables/ip6tables.c $(PKG_BUILD_DIR)/
	$(CP) $(IPTABLES_PATH)/iptables/xshared.c $(PKG_BUILD_DIR)/
	$(CP) $(IPTABLES_PATH)/iptables/xshared.h $(PKG_BUILD_DIR)/
endef

define Package/firewall_uci/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/fw_iface_stats.sh $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/firewall_uci.init $(1)/etc/init.d/firewall
	$(INSTALL_DATA) ./files/firewall.config $(1)/etc/config/firewall
	$(INSTALL_BIN) ./files/firewall.hotplug $(1)/etc/hotplug.d/iface/20-firewall
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iptables-uci $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ip6tables-uci $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,firewall_uci))
