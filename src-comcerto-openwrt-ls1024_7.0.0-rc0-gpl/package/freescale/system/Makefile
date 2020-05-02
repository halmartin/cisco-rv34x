# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=system
PKG_VERSION:=fsl
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/system
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=system
	DEPENDS:=+libuci
endef

define Package/system/description
	system base module.
endef

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
	-DSPI_OPERATION \
	-D_LITTLE_ENDIAN

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR)/src -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/system/install
	echo "system module install !!"
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/system.init $(1)/etc/init.d/system
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) files/exec_wanscript.py $(1)/usr/bin/
	$(INSTALL_BIN) files/syserr_codes $(1)/usr/bin/
	$(INSTALL_BIN) files/logread $(1)/usr/bin/
	$(INSTALL_BIN) files/certificate.init $(1)/etc/init.d/certificate
	$(INSTALL_BIN) files/certsInfraMigration.sh $(1)/usr/bin/
	$(INSTALL_BIN) files/pkt-capture-wrapper $(1)/usr/bin/
	$(INSTALL_BIN) files/systemconfig.init $(1)/etc/init.d/systemconfig
	$(INSTALL_BIN) files/bootwrapper.init $(1)/etc/init.d/bootwrapper
	$(INSTALL_BIN) files/bootwrapper2.init $(1)/etc/init.d/bootwrapper2
	$(INSTALL_DATA) files/systemconfig.config $(1)/etc/config/systemconfig
	$(INSTALL_DATA) files/systeminfo.config $(1)/etc/config/systeminfo
	$(INSTALL_DATA) files/systemstats.config $(1)/etc/config/systemstats
	$(INSTALL_BIN) files/systemstats.init $(1)/etc/init.d/systemstats
	$(INSTALL_BIN) files/finish.init $(1)/etc/init.d/zfinish
	$(INSTALL_BIN) files/lanwancheck.init $(1)/etc/init.d/lanwancheck
	$(INSTALL_BIN) files/getSystemStats $(1)/usr/bin/getSystemStats
	$(INSTALL_BIN) files/schedule.init $(1)/etc/init.d/schedule
	$(INSTALL_BIN) files/ipgroup.init $(1)/etc/init.d/ipgroup
	$(INSTALL_BIN) files/SwinfoUpdate $(1)/etc/init.d/
	$(INSTALL_DATA) files/schedule.config $(1)/etc/config/schedule
	$(INSTALL_DATA) files/ipgroup.config $(1)/etc/config/ipgroup
	$(INSTALL_BIN) files/updateCertUsage $(1)/usr/bin/
	$(INSTALL_BIN) files/rpc/* $(1)/usr/bin/
	$(INSTALL_BIN) files/install_usb_drivers $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/command
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/1minute
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/5minutes
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/10minutes
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/hourly
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/daily
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/weekly
	$(INSTALL_DIR) $(1)/usr/bin/cron.d/monthly
	$(INSTALL_BIN) files/crond/files/* $(1)/usr/bin/cron.d/command/
	$(INSTALL_DATA) files/crond/config/crontabs.config $(1)/etc/config/crontabs
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/get_timezoneday $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/check_dsttime $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/checkDst $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/dhcp_release $(1)/usr/bin/
	$(INSTALL_BIN) files/ntpd_update $(1)/usr/bin/
	#$(INSTALL_BIN) files/set_dst_time.sh $(1)/usr/bin/
	$(INSTALL_BIN) files/check_usbdevices $(1)/usr/bin/
	$(INSTALL_BIN) files/dstHandler $(1)/usr/bin/
	$(INSTALL_BIN) files/1-mwan3OnDemand.sh $(1)/usr/bin/cron.d/1minute/
	$(INSTALL_BIN) files/1-healthMonitor.sh $(1)/usr/bin/cron.d/5minutes/
	$(INSTALL_BIN) files/2-usage_monitor $(1)/usr/bin/cron.d/10minutes/
	$(INSTALL_BIN) files/updateFlashTime $(1)/usr/bin/
	$(INSTALL_DATA) files/mnt_flag $(1)/etc/
	$(INSTALL_BIN) files/delete_certificates $(1)/usr/bin/
	$(INSTALL_BIN) files/generate_default_cert $(1)/usr/bin/
	$(INSTALL_BIN) files/blink_usb1.sh $(1)/usr/bin/
	$(INSTALL_BIN) files/blink_usb2.sh $(1)/usr/bin/
	$(INSTALL_BIN) files/bootup_Upgrade $(1)/usr/bin/
	$(INSTALL_BIN) files/ifupUsbDevice $(1)/usr/bin/
ifeq ($(CONFIG_TARGET_comcerto2000_hgw_c2krv340), y)
	$(RM) $(1)/usr/bin/generate-openvpn-client-template
	$(RM) $(1)/usr/bin/terminate-openvpn-client
	$(INSTALL_DATA) files/boardInfo.rv340 $(1)/etc/boardInfo
else
	$(INSTALL_DATA) files/boardInfo $(1)/etc/boardInfo
endif
	$(INSTALL_DIR) $(1)/mnt/configcert/config
	$(INSTALL_DATA) files/rebootstate $(1)/mnt/configcert/config
	$(INSTALL_BIN) files/2-syslog_hourly $(1)/usr/bin/cron.d/hourly/
	$(INSTALL_BIN) files/2-syslog_daily $(1)/usr/bin/cron.d/daily/
	$(INSTALL_BIN) files/2-syslog_weekly $(1)/usr/bin/cron.d/weekly/

endef

$(eval $(call BuildPackage,system))
