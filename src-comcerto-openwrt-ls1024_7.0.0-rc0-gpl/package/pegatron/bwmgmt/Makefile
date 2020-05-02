#
# for pegatron bandwidth management api
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bwmgmt
PKG_VERSION:=0.0.2
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/bwmgmt
  SECTION:=Pegatron
  CATEGORY:=Pegatron Packages
  DEPENDS:=+libnl-tiny
  TITLE:=BandWidth Management
endef

TARGET_CFLAGS += \
        -I$(STAGING_DIR)/usr/include/libnl-tiny \
        -I$(STAGING_DIR)/usr/include

CMAKE_OPTIONS += \
        -DLIBNL_LIBS=-lnl-tiny \
        -DDEBUG=1

define Build/Prepare

endef

define Build/Compile

endef

define Package/bwmgmt/install
	$(CP) ./files/* $(1)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/bwmgmtd $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/reset-mobile-statistics $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/bin/bwmgmt-trigger $(1)/usr/bin/
endef

$(eval $(call BuildPackage,bwmgmt))

