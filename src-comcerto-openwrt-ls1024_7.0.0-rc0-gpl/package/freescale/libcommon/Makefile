# 
#
#
#
# 
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcommon
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/libcommon
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=LibCommon
endef

define Package/libcommon/description
	Common modules
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/libcommon/install
endef

define Build/InstallDev
	install -d -m0755 $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/arrsaferef.h $(1)/usr/include/
endef

define Build/UninstallDev
	$(RM) $(STAGING_DIR)/usr/include/arrsaferef.h
endef

$(eval $(call BuildPackage,libcommon))
