# 
#
#
#
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bccsdk
PKG_VERSION:=0.7
#PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
#PKG_SOURCE:=$(PKG_NAME).tgz

include $(INCLUDE_DIR)/package.mk

PKG_BIN_SRC=$(TOPDIR)/dl-bin/$(PKG_NAME)-$(PKG_VERSION).tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/bccsdk
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	DEFAULT:=n
	TITLE:=bccsdk(Bright Cloud Services SDK)
	DEPENDS:=+libopenssl
endef

define Package/bccsdk/description
	BCCSDK library for use by Webfiltering module.
endef

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
	-DSPI_OPERATION \
	-D_LITTLE_ENDIAN

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
endef
else
define Build/Prepare
	$(call Build/Prepare/Default)
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
endef
else
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h $(TARGET_CFLAGS_EXTRA)"
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Package/bccsdk/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/bcap $(1)/usr/sbin/
endef
else
define Package/bccsdk/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/bcap $(1)/usr/sbin/
endef
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_BUILD_DIR)/src/libbccsdk.a $(1)/usr/lib/
endif

$(eval $(call BuildPackage,bccsdk))
