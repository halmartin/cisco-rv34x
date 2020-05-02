# 
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qos_uci
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

IPTABLES_PATH:=$(KERNEL_BUILD_DIR)/iptables-*/

include $(INCLUDE_DIR)/package.mk

define Package/qos_uci
  CATEGORY:=Freescale Packages
  SECTION:=Freescale Packages
  TITLE:=UCI based iptables qos 
  URL:=http://www.freescale.com/
  DEPENDS:=+libuci +libubus +libxtables +iptables +ip6tables
endef

define Package/qos_uci/description
 This package is for UCI based iptables qos 
endef

#MAKE_FLAGS += \
	UCIMAPLIB="$(STAGING_DIR)/usr/lib/libucimap.a $(STAGING_DIR)/usr/lib/libip4tc.so $(STAGING_DIR)/usr/lib/libxtables.so $(IPTABLES_PATH)extensions/libiptext4.so $(IPTABLES_PATH)extensions/libiptext.so"

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

define Package/qos_uci/install
	$(INSTALL_DIR) $(1)
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) ./files/* $(1)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qos-iptables-uci $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qos-ip6tables-uci $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,qos_uci))
