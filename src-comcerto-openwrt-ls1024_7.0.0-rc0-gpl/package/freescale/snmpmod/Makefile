# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
#include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=snmpmod
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

CONFD_PKG=confd-6.4.4
CONFD_PKG_BUILD_DIR=$(BUILD_DIR)/$(CONFD_PKG)

PKG_BUILD_DEPENDS:=confd

include $(INCLUDE_DIR)/package.mk

PKG_BIN_SRC=$(TOPDIR)/dl-bin/snmpmod-fsl.1.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/snmpmod
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=snmpmod
	DEPENDS:=+confd +libuci
endef

define Package/snmpmod/description
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
	$(CP) -a files $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/files -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

else
define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
	$(CP) -a files $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/files -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include" \
	CONFD_LIB="$(CONFD_PKG_BUILD_DIR)/lib/libconfd.so" \
	LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"
endef

else

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA) -I$(PKG_BUILD_DIR)/src/header -I$(CONFD_PKG_BUILD_DIR)/include" \
	CONFD_LIB="$(CONFD_PKG_BUILD_DIR)/lib/libconfd.so" \
	LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/src"
endef

endif

ifeq ($(FILE_EXISTS), 1)
define Package/snmpmod/install
	echo "snmpmod module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/snmpglue $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/procdata.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ciscopvtmib_common.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/getwandns.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/getiface.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/gettrapsconf.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_lldp_neighinfo.sh $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp.init $(1)/etc/init.d/snmpinit 
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/snmp.hotplug $(1)/etc/hotplug.d/iface/53-snmp
	$(INSTALL_DIR) $(1)/etc/confd/cdb
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_init.xml $(1)/etc/confd/cdb 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_vacm_init.xml $(1)/etc/confd/cdb 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_notify_init.xml $(1)/etc/confd/cdb 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_target_init.xml $(1)/etc/confd/cdb 
	#$(INSTALL_DIR) $(1)/tmp/etc/config
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp.conf $(1)/tmp/etc/config/snmpconf
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_community_init.xml $(1)/etc/confd/cdb 
endef
else
define Package/snmpmod/install
	echo "snmpmod module install !!"
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/snmpglue $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/procdata.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ciscopvtmib_common.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/getwandns.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/getiface.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/gettrapsconf.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_lldp_neighinfo.sh $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp.init $(1)/etc/init.d/snmpinit 
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/snmp.hotplug $(1)/etc/hotplug.d/iface/53-snmp
	$(INSTALL_DIR) $(1)/etc/confd/cdb
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_init.xml $(1)/etc/confd/cdb 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_vacm_init.xml $(1)/etc/confd/cdb 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_notify_init.xml $(1)/etc/confd/cdb 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_target_init.xml $(1)/etc/confd/cdb 
	#$(INSTALL_DIR) $(1)/tmp/etc/config
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp.conf $(1)/tmp/etc/config/snmpconf
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/snmp_community_init.xml $(1)/etc/confd/cdb 
endef
endif
	#$(INSTALL_DIR) $(1)/lib
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/libsnmpmod.so $(1)/lib/

$(eval $(call BuildPackage,snmpmod))
