include $(TOPDIR)/rules.mk

PKG_NAME:=usb-modem-mgmt
PKG_VERSION=1.0.00.01
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/usb-modem-mgmt
  SECTION:=Pegatron
  CATEGORY:=Pegatron Packages
  TITLE:=3G/4G USB Modem Management Application
endef

define Build/Prepare
	@true
endef

define Build/Compile
	@true
endef

define Package/usb-modem-mgmt/description
  3G/4G USB Modem Management Application
endef

define Package/usb-modem-mgmt/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,usb-modem-mgmt))
