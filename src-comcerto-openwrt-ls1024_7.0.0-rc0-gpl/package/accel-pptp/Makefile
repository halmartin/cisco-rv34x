include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=accel-pptp
PKG_VERSION:=0.8.5
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/accel-pptp
PKG_MD5SUM:=061ca3991fb69e0e79cb872b0aaf1d89

# PPPD version we build for
PPPD_VERSION:=2.4.5

include $(INCLUDE_DIR)/package.mk

define Package/ppp-mod-pptp
	SECTION:=net
	CATEGORY:=Network
	TITLE:=ACCEL-PPTP plugin
	URL:=http://accel-pptp.sourceforge.net/
	DEPENDS:=@(PACKAGE_ppp||PACKAGE_ppp-multilink) @!PACKAGE_pptp +kmod-pptp
endef

define Package/ppp-mod-pptp/description
	High performance PPTP plugin using kernel support.
endef

define Package/ppp-mod-pptp/conffiles
	/etc/ppp/options.pptp
endef

CONFIGURE_ARGS += --disable-static
CONFIGURE_VARS += \
	KDIR="$(KERNEL_BUILD_DIR)/linux-4.1.8" \
	PPPD_VERSION="$(PPPD_VERSION)"

# we patch configure.in, so do autoconf
define Build/Prepare
	$(call Build/Prepare/Default)
	autoreconf "$(PKG_BUILD_DIR)/pppd_plugin" -f -i
	autoreconf "$(PKG_BUILD_DIR)/pptpd-1.3.3" -f -i
endef

define Build/Configure
	$(call Build/Configure/Default,,,pppd_plugin)
	$(call Build/Configure/Default,,,pptpd-1.3.3)
endef

define Build/ppp-mod-pptp/Compile
	cd $(KERNEL_BUILD_DIR)/accel-pptp-0.8.5/ && tar -zcvf pptpd-1.3.3.tar.gz pptpd-1.3.3 && cp pptpd-1.3.3.tar.gz $(TOPDIR)/dl/ && cd -;
	$(call Build/Compile/Default,-C pppd_plugin)
endef

define Build/Compile
	$(call Build/ppp-mod-pptp/Compile)
endef

define Package/ppp-mod-pptp/install
	$(INSTALL_DIR) $(1)/usr/lib/pppd/$(PPPD_VERSION)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pppd_plugin/src/.libs/pptp.so.0.0.0 \
				   $(1)/usr/lib/pppd/$(PPPD_VERSION)/pptp.so
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_DATA) ./files/options.pptp $(1)/etc/ppp/
	$(INSTALL_DIR) $(1)/lib/network
	$(INSTALL_BIN) ./files/pptp.sh $(1)/lib/network/
endef

$(eval $(call BuildPackage,ppp-mod-pptp))
