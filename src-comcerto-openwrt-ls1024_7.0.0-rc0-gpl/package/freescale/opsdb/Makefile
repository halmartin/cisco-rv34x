# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
#include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=opsdb
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

CONFD_PKG=confd-6.4.4
CONFD_PKG_BUILD_DIR=$(BUILD_DIR)/$(CONFD_PKG)

# Cisco yang header files path inclusion from the ciscosb-yang package directly
#CISCOSB_YANG_HEADERS_PATH=$(BUILD_DIR)/../../package/cisco/ciscosb-yang/files

PKG_BUILD_DEPENDS:=confd ciscosb-yang

include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl-bin/opsdb-fsl.1.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/opsdb
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=opsdb
	DEPENDS:=+confd +ciscosb-yang +libsqlite3
endef

define Package/opsdb/description
	Operation data statistics module.
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
	$(call Build/Prepare/Default)
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

else

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include  -I$(CISCOSB_YANG_HEADERS_PATH)/header" \
	CONFD_LIB="$(CONFD_PKG_BUILD_DIR)/lib/libconfd.so" \
	LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"
endef

else
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include  -I$(CISCOSB_YANG_HEADERS_PATH)/header" \
	CONFD_LIB="$(CONFD_PKG_BUILD_DIR)/lib/libconfd.so" \
	LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Package/opsdb/install
	echo "opsdb module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) files/get_dhcp_bind_status $(1)/usr/bin/
	$(INSTALL_BIN) files/pptpscript $(1)/usr/bin/
	$(INSTALL_BIN) files/l2tpscript $(1)/usr/bin/
	$(INSTALL_BIN) files/sitetositescript $(1)/usr/bin/
	$(INSTALL_BIN) files/clienttositescript $(1)/usr/bin/
	$(INSTALL_BIN) files/teleworkerscript $(1)/usr/bin/
	$(INSTALL_BIN) files/neighscriptv6 $(1)/usr/bin/
	$(INSTALL_BIN) files/neighscriptv4 $(1)/usr/bin/
	$(INSTALL_BIN) files/wanscript $(1)/usr/bin/
	$(INSTALL_BIN) files/wangetcurrent $(1)/usr/bin/
	$(INSTALL_BIN) files/executeWanScript $(1)/usr/bin/
	$(INSTALL_BIN) files/routescript $(1)/usr/bin/
	$(INSTALL_BIN) files/qosscript $(1)/usr/bin/
	$(INSTALL_BIN) files/get_upnp_rules $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/liboperdata_api.so $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/operdb_stats $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DATA) files/services_list $(1)/etc/
	$(INSTALL_BIN) files/opsdb.init $(1)/etc/init.d/opsdb
	$(INSTALL_BIN) files/tcpipservicesscript $(1)/usr/bin/
	$(INSTALL_BIN) files/certscript $(1)/usr/bin/
	$(INSTALL_BIN) files/certscriptCtrl $(1)/usr/bin/
	$(INSTALL_BIN) files/get_lan_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/get_wan_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/get_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/get_wlan_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/if_radio_get_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/if_ssid_get_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/vap_get_info $(1)/usr/bin/
	$(INSTALL_BIN) files/ssids_load_stats $(1)/usr/bin/
	$(INSTALL_BIN) files//cps_load_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/aps_load_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/lldp_neighscript $(1)/usr/bin/
	$(INSTALL_BIN) files/connect_detect.sh $(1)/usr/bin/
	$(INSTALL_BIN) files/get_connections $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/10minutes
	$(INSTALL_BIN) files/sendOpsdbSignal.sh $(1)/usr/bin/
ifeq ($(CONFIG_TARGET_comcerto2000_hgw_c2krv16x-26x),y)
	$(INSTALL_BIN) files/openvpnscript $(1)/usr/bin/
endif
	$(INSTALL_BIN) files/get_switch_qos_stats $(1)/usr/bin/
	ln -sf /mnt/packages/mobile/usr/bin/update_mobile.sh $(1)/usr/bin/update_mobile.sh
endef
else
define Package/opsdb/install
	echo "opsdb module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) files/get_dhcp_bind_status $(1)/usr/bin/
	$(INSTALL_BIN) files/pptpscript $(1)/usr/bin/
	$(INSTALL_BIN) files/l2tpscript $(1)/usr/bin/
	$(INSTALL_BIN) files/sitetositescript $(1)/usr/bin/
	$(INSTALL_BIN) files/clienttositescript $(1)/usr/bin/
	$(INSTALL_BIN) files/teleworkerscript $(1)/usr/bin/
	$(INSTALL_BIN) files/neighscriptv6 $(1)/usr/bin/
	$(INSTALL_BIN) files/neighscriptv4 $(1)/usr/bin/
	$(INSTALL_BIN) files/wanscript $(1)/usr/bin/
	$(INSTALL_BIN) files/wangetcurrent $(1)/usr/bin/
	$(INSTALL_BIN) files/executeWanScript $(1)/usr/bin/
	$(INSTALL_BIN) files/routescript $(1)/usr/bin/
	$(INSTALL_BIN) files/qosscript $(1)/usr/bin/
	$(INSTALL_BIN) files/get_upnp_rules $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/liboperdata_api.so $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/operdb_stats $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DATA) files/services_list $(1)/etc/
	$(INSTALL_BIN) files/opsdb.init $(1)/etc/init.d/opsdb
	$(INSTALL_BIN) files/tcpipservicesscript $(1)/usr/bin/
	$(INSTALL_BIN) files/certscript $(1)/usr/bin/
	$(INSTALL_BIN) files/certscriptCtrl $(1)/usr/bin/
	$(INSTALL_BIN) files/get_lan_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/get_wan_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/get_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/get_wlan_port_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/if_radio_get_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/if_ssid_get_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/vap_get_info $(1)/usr/bin/
	$(INSTALL_BIN) files/ssids_load_stats $(1)/usr/bin/
	$(INSTALL_BIN) files//cps_load_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/aps_load_stats $(1)/usr/bin/
	$(INSTALL_BIN) files/lldp_neighscript $(1)/usr/bin/
	$(INSTALL_BIN) files/connect_detect.sh $(1)/usr/bin/
	$(INSTALL_BIN) files/get_connections $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/10minutes
	$(INSTALL_BIN) files/sendOpsdbSignal.sh $(1)/usr/bin/
ifeq ($(CONFIG_TARGET_comcerto2000_hgw_c2krv16x-26x),y)
	$(INSTALL_BIN) files/openvpnscript $(1)/usr/bin/
endif
	$(INSTALL_BIN) files/get_switch_qos_stats $(1)/usr/bin/
	ln -sf /mnt/packages/mobile/usr/bin/update_mobile.sh $(1)/usr/bin/update_mobile.sh
endef
endif

$(eval $(call BuildPackage,opsdb))
