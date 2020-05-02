# 
#
#
#
#
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=epavis-dpi
PKG_VERSION:=devel.mainline
PKG_RELEASE:=20180130

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/epavis-dpi
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=epavis dpi driver
  DEPENDS:= +libuci +libcurl +libncurses +libreadline +libxml2 +kmod-ipt-conntrack +smart_client
endef

define Package/epavis-dpi/description
  Lionic DPI modules for RV34X
endef

define Build/Configure
endef

define Build/Compile
	echo "Lionic Ysmir package Copying !!"
endef

define Package/epavis-dpi/install
	echo "Lionic Ysmir package install !!"
	$(CP) ./files/root $(1)/
	$(CP) ./files/etc $(1)/
	# The schedule and ipgroup configuration have to be used from system-management. Hence removing them from copying here
	$(RM) $(1)/etc/config/schedule
	$(RM) $(1)/etc/config/ipgroup
	$(RM) $(1)/etc/config/usrgroup
	$(CP) ./files/lib $(1)/
	$(CP) ./files/usr $(1)/
	$(INSTALL_BIN) files/rpc/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/lionic.hotplug $(1)/etc/hotplug.d/iface/26-lionic
	$(INSTALL_BIN) ./files/avc-metadata.sh   $(1)/usr/bin/
	$(INSTALL_BIN) ./files/avc-deviceos-metadata.sh   $(1)/usr/bin/
endef

#	$(CP) ./files/lcsig.sh       $(1)/root/lionic/

$(eval $(call BuildPackage,epavis-dpi))
