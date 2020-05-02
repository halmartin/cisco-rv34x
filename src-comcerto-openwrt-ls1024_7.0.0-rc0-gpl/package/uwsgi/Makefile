#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uwsgi
PKG_VERSION:=2.0.15
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://projects.unbit.it/downloads/
PKG_MD5SUM:=fc50bd9e83b7602fa474b032167010a7
PKG_MAINTAINER:=Ackman Liang <Ackman_Liang@pegatroncorp.com>
PKG_LICENSE:=GPLv2

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
	
define Package/uwsgi
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Application Server
  DEPENDS:=+libopenssl +libpthread +libxml2 +zlib +libpcre +libuuid
  MENU:=1
endef

define Package/uwsgi/description
 Application servers (for various programming languages and protocols), proxies, process managers and monitors are all implemented using a common api and a common configuration style.
endef


define Package/uwsgi/conffiles
/etc/uwsgi/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CPP="$(TARGET_CXX)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	UWSGI_INCLUDES="$(STAGING_DIR)/usr/include" \
	UWSGI_PLUGIN_DIR="/usr/lib/uwsgi/plugins" \
	ciscosbr
endef

define Build/Install
endef

define Package/uwsgi/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uwsgi $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/uwsgi/plugins
	$(CP) $(PKG_BUILD_DIR)/*_plugin.so $(1)/usr/lib/uwsgi/plugins
	$(INSTALL_DIR) $(1)/etc/uwsgi
	$(INSTALL_DATA) ./files/etc/uwsgi/*.ini $(1)/etc/uwsgi/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/uwsgi.init $(1)/etc/init.d/uwsgi
endef

$(eval $(call BuildPackage,uwsgi))

