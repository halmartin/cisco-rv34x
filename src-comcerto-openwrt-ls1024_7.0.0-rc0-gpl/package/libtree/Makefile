include $(TOPDIR)/rules.mk

PKG_NAME := libtree
PKG_VERSION := 1.0.0
PKG_RELEASE := 1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/fbuihuu/libtree.git
PKG_SOURCE_VERSION:=HEAD
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE := $(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/libtree
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libtree
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared

define Package/libtree/install
	$(INSTALL_DIR) $(1)/usr/include/libtree
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtree))
