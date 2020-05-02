#
# Copyright (C) 2016-2017
#
# This is free software, licensed under the GNU General Public License v2.
# 

include $(TOPDIR)/rules.mk

PKG_NAME:=cportal
PKG_VERSION:=devel.mainline
PKG_RELEASE:=2019-04-16_00_00__0530

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE).tar.gz
PKG_CAT:=zcat

include $(INCLUDE_DIR)/package.mk

define Package/cportal	
  SECTION:=net
  CATEGORY:=Captive Portal
  DEFAULT:=y
  TITLE:=Captive Portal
  DEPENDS := +libopenssl +libtree +libpam +libnetfilter-queue
endef

define Package/cportal/description
	Captive Portal
endef

TARGET_CFLAGS += \
	-I$(BUILD_DIR)/libtree \
	-I$(STAGING_DIR)/usr/include/

TARGET_LDFLAGS += \
	-L$(STAGING_DIR)/root-comcerto2000/usr/lib -ltree -lssl -lcrypto 

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Build/Configure
endef

define Package/cportal/config
endef

ifeq ($(CONFIG_DEV_CPORTAL), y)
define Build/Prepare
	pushd "$(BUILD_DIR)" && \
	git clone -b $(CONFIG_DEV_CPORTAL_GITBRANCH) ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_CPORTAL_GITTREE) $(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE) && \
	popd
	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
        $(KPATCH) $(PKG_BUILD_DIR) ./patches; \
    fi
endef
endif


define Build/Compile
	$(call Build/Compile/Default, \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"\
		DESTDIR="$(PKG_INSTALL_DIR)" \
		INSTALL="install" \
		all install \
	)
endef

define Package/cportal/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/captive_portal $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/cportald $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/cportalctl $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,cportal,libopenssl,libtree,libpam,libnetfilter-queue))
