include $(TOPDIR)/rules.mk

PKG_NAME:=sbr-gui
PKG_VERSION:=1.0.2.10
PKG_RELEASE:=8
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)$(if $(PKG_RELEASE),-$(PKG_RELEASE)).tar.xz

include $(INCLUDE_DIR)/package.mk

define Package/sbr-gui
  SECTION:=Pegatron
  CATEGORY:=Pegatron Packages
  DEPENDS:=+confd +libjson-c +libuci +libwebsockets +libpcre
  TITLE:=Cisco SBR GUI
endef

define Package/sbr-gui/description
 Pegatron Release.
endef

define Build/Prepare
	@true
endef

define Build/Compile
	@true
endef

define Package/sbr-gui/install
	$(TAR) -Jxf $(DL_DIR)/$(PKG_SOURCE) -C$(1)
endef

$(eval $(call BuildPackage,sbr-gui))
