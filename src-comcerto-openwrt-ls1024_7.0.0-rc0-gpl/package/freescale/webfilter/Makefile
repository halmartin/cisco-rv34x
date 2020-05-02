# 
#
#
#
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webfilter
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

BCCSDK_PKG=bccsdk-0.7
BCCSDK_PKG_BUILD_DIR=$(BUILD_DIR)/$(BCCSDK_PKG)/src

EPAVIS_WCFINFRA=$(TOPDIR)/package/freescale/epavis

include $(INCLUDE_DIR)/package.mk

PKG_BIN_SRC=$(TOPDIR)/dl-bin/webfilter-fsl.1.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/webfilter
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=Webfilter
	DEPENDS:= +epavis-dpi +bccsdk +libopenssl +libcommon
endef

define Package/webfilter/description
	Webfiltering application leveraging BCCSDK API from Webroot and WCF infra from LIONIC.
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
	$(CP) -a src $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	echo "Webfilter module using binary tarball from dl-bin !!"
endef
else
define Build/Compile
	$(CP) $(EPAVIS_WCFINFRA)/src/lc_api.c $(PKG_BUILD_DIR)/src/
	$(CP) $(EPAVIS_WCFINFRA)/src/lc_api.h $(PKG_BUILD_DIR)/src/
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/include-host -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(BCCSDK_PKG_BUILD_DIR)" \
	BCCSDK_LIB=$(BCCSDK_PKG_BUILD_DIR)/libbccsdk.a \
	LDFLAGS="$(TARGET_LDFLAGS) "
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Package/webfilter/install
	echo "Webfilter module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wfapp $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wfcli $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wf_agent $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/url_lookup $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/webfilter.init   $(1)/etc/init.d/webfilter
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/webfilter.config $(1)/etc/config/webfilter
	$(INSTALL_BIN) ./files/webfilter-metadata.sh   $(1)/usr/bin/
	$(INSTALL_BIN) ./files/url-lookup.rpc $(1)/usr/bin/url-lookup
endef
else
define Package/webfilter/install
	echo "Webfilter module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wfapp $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wfcli $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wf_agent $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/url_lookup $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/webfilter.init   $(1)/etc/init.d/webfilter
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/webfilter.config $(1)/etc/config/webfilter
	$(INSTALL_BIN) ./files/webfilter-metadata.sh   $(1)/usr/bin/
	$(INSTALL_BIN) ./files/url-lookup.rpc $(1)/usr/bin/url-lookup
endef
endif

$(eval $(call BuildPackage,webfilter))
