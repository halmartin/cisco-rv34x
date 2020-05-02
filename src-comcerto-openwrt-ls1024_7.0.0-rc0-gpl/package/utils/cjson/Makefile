# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cjson
PKG_VERSION:=0.1
PKG_SOURCE_URL:=https://sourceforge.net/projects/cjson/files/cJSONFiles.zip
PKG_SOURCE:=cJSONFiles.zip
PKG_MD5SUM:=4ff46c0e451904951019597da78df7c1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_WORKSPACE_DIR:=$(PKG_BUILD_DIR)/cJSON

include $(INCLUDE_DIR)/package.mk

define Package/cjson
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Parser to parse JSON messages.
endef

define Package/cjson/description
	cjson parser to parse JSON messages.
endef


MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS=" $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Build/Prepare
	unzip $(DL_DIR)/$(PKG_SOURCE) -d $(PKG_BUILD_DIR)
	$(Build/Patch)
	echo "prepare done..."
endef

define Build/Compile
	$(MAKE) -C $(PKG_WORKSPACE_DIR)/   $(TARGET_CONFIGURE_OPTS) LIBS="$(TARGET_LDFLAGS) -lm"
endef

define Package/cjson/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_WORKSPACE_DIR)/libcJSON.so $(1)/lib/
endef

$(eval $(call BuildPackage,cjson))
