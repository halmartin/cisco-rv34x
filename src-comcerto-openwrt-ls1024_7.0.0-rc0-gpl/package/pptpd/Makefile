# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pptpd
PKG_VERSION:=1.3.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=@SF/poptop
#PKG_MD5SUM:=75d494e881f7027f4e60b114163f6b67 #1.3.0
PKG_SOURCE_FILE:=$(TOPDIR)/dl/$(PKG_SOURCE)
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

PPPD_VERSION:=2.4.5

define Package/pptpd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-ppp +kmod-gre +ppp
  TITLE:=PopTop pptp server(Accel-pptp)
  URL:=http://www.poptop.org/
  SUBMENU:=VPN
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	autoreconf "$(PKG_BUILD_DIR)" -f -i
endef

CONFIGURE_ARGS += --disable-static
CONFIGURE_VARS += \
	KDIR="$(KERNEL_BUILD_DIR)/linux-4.1.8" \
	PPPD_VERSION="$(PPPD_VERSION)"

define Build/Configure
	$(call Build/Configure/Default, \
		--enable-bcrelay KDIR="$(KERNEL_BUILD_DIR)/linux-4.1.8" PPPD_VERSION="$(PPPD_VERSION)" \
	)
endef

define Build/Compile
	rm -vf $(PKG_BUILD_DIR)/if_pppox.h ; cp -fv $(KERNEL_BUILD_DIR)/accel-pptp-0.8.5/kernel/driver/if_pppox.h $(PKG_BUILD_DIR)/if_pppox.h;
	$(call Build/Compile/Default, \
		COPTS="$(TARGET_CFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		INSTALL="install" \
		all install \
	)
endef

define Package/pptpd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/pptpd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/pptpctrl $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib/pptpd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pptpd/* $(1)/usr/lib/pptpd
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/pptpd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pptpd.init $(1)/etc/init.d/pptpd
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_DATA) ./files/options.pptpd $(1)/etc/ppp/
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/pptpd.config $(1)/etc/config/pptpd
endef

$(eval $(call BuildPackage,pptpd))
