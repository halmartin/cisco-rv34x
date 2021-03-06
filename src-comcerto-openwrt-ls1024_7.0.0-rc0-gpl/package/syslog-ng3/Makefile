#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=syslog-ng
PKG_VERSION:=3.0.8
PKG_RELEASE:=5

PKG_SOURCE_URL:=http://www.balabit.com/downloads/files/syslog-ng/open-source-edition/$(PKG_VERSION)/source/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_MD5SUM:=7107f5758dec4b774136f0f827b35258

PKG_CONFIG_DEPENDS:= \
	CONFIG_IPV6 \

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/syslog-ng3
 SECTION:=admin
 CATEGORY:=Administration
 DEPENDS:=+libpcre +glib2 +libeventlog +libopenssl +libwrap +libpthread +librt +zlib +libdbi
 TITLE:=A powerful syslog daemon
 URL:=http://www.balabit.com/network-security/syslog-ng/opensource-logging-system/
endef

define Package/syslog-ng3/description
 syslog-ng reads and logs messages to the system console, log
 files, other machines and/or users as specified by its
 configuration file.
endef

define Package/syslog-ng3/conffiles
/etc/syslog-ng.conf
endef

define Build/Configure
	$(SED) 's,-I/usr/include,,' $(PKG_BUILD_DIR)/configure
	$(Build/Configure/Default)
endef

CONFIGURE_ARGS += \
	$(call autoconf_bool,CONFIG_IPV6,ipv6) \
	--disable-dependency-tracking \
	--enable-tcp-wrapper \
	--disable-glibtest \
	--disable-spoof-source \
	--disable-sql \
	--enable-pcre \
	--disable-linux-caps \

TARGET_CPPFLAGS += \
	-I$(STAGING_DIR)/usr/include/eventlog

CONFIGURE_VARS += \
	LIBDBI_CFLAGS="-I$(STAGING_DIR)/usr/include"

define Package/syslog-ng3/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/syslog-ng $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/sendEmailAlert $(1)/usr/bin/
	$(INSTALL_BIN) ./files/sendEmailLogs $(1)/usr/bin/
	$(INSTALL_BIN) ./files/logInterface $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/syslog.init $(1)/etc/init.d/syslog
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/syslog.config $(1)/etc/config/syslog
	$(INSTALL_DATA) ./files/syslog-ng.conf $(1)/etc/syslog_config_template
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/52-interface_logs.hotplug $(1)/etc/hotplug.d/iface/52-interface_logs
	$(INSTALL_BIN) ./files/sendmaillog $(1)/usr/bin/
	$(INSTALL_BIN) ./files/sendmailalert $(1)/usr/bin/
endef

$(eval $(call BuildPackage,syslog-ng3))
