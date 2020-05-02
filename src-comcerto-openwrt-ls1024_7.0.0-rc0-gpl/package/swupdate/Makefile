# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=swupdate
PKG_VERSION:=0.1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=curl

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/swupdate-0.1.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif


define Package/swupdate
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  DEPENDS:=+curl +cjson +libopenssl
  TITLE:=Software update utility
endef

define Package/swupdate/description
	swupdate application for automatic software update checks.
endef

#TARGET_CPPFLAGS:= $(TARGET_CPPFLAGS)

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS=" $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

ifeq ($(CONFIG_TARGET_RUSSIA_IMAGE), y)
	TARGET_CFLAGS+=-DRU_IMAGE
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	$(call Build/Prepare/Default)
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
	$(CP) -a files $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

else

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)
	$(CP) -a files $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I $(BUILD_DIR)/cjson/cJSON" \
		$(TARGET_CONFIGURE_OPTS) \
		LIBS="$(TARGET_LDFLAGS) -L$(BUILD_DIR)/cjson/cJSON -lm -lcurl"
endef

else

define Build/Compile
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I $(BUILD_DIR)/cjson/cJSON" \
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		$(TARGET_CONFIGURE_OPTS) \
		LIBS="$(TARGET_LDFLAGS) -L$(BUILD_DIR)/cjson/cJSON -lm -lcurl"
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Package/swupdate/install
	mkdir -p $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/sw_dwnlod_apply.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/swupdate_chk.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/compswvers.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/sendAsdmail $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/updateswinfo.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/src/asdclient $(1)/usr/bin/
	$(INSTALL_DIR)  $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/swupdate.init $(1)/etc/init.d/swupdateinit
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) files/swupdate.config $(1)/etc/config/swupdate
	$(INSTALL_BIN) files/verchk $(1)/etc/verchk
endef

else
define Package/swupdate/install
	mkdir -p $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/sw_dwnlod_apply.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/swupdate_chk.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/compswvers.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/sendAsdmail $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/files/updateswinfo.sh $(1)/usr/bin/
	$(CP) -a $(PKG_BUILD_DIR)/src/asdclient $(1)/usr/bin/
	$(INSTALL_DIR)  $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/swupdate.init $(1)/etc/init.d/swupdateinit
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) files/swupdate.config $(1)/etc/config/swupdate
	$(INSTALL_BIN) files/verchk $(1)/etc/verchk
endef
endif

$(eval $(call BuildPackage,swupdate))
