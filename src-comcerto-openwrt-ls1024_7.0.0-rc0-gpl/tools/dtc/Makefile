#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dtc
PKG_VERSION:=1.4.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=git://git.kernel.org/pub/scm/utils/dtc/dtc.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=302fca9f4c283e1994cf0a5a9ce1cf43ca15e6d2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_CAT:=bzcat
PKG_BUILD_DIR=$(BUILD_DIR_HOST)/$(PKG_NAME)

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/dtc $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/dtc
endef

$(eval $(call HostBuild))
