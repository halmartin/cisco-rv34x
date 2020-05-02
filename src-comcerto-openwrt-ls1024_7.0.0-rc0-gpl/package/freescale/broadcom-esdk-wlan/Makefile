# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include ./pkg_config.mk

PKG_NAME:=broadcom_esdk_wlan
PKG_BROADCOM_VERSION:=5.1.4.1
ifeq ($(CONFIG_DEV_BROADCOM_ESDK_WLAN), y)
PKG_BROADCOM_VERSION:=$(shell (echo ${CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH} | grep -P -o '(\d+_)+\d+' | sed 's/_/./g'))
$(info $$PKG_BROADCOM_VERSION is [${PKG_BROADCOM_VERSION}])
$(info $$CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH is [${CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH}])
endif
PKG_MSPD_VERSION:=5.00
PKG_RELEASE:=3
PKG_VERSION:=$(PKG_BROADCOM_VERSION)-$(PKG_MSPD_VERSION).$(PKG_RELEASE)

ifeq ($(CONFIG_TARGET_comcerto2000_hgw_c2krv340),y)
PLT=rv340w
else
PLT=rv160w_260w
endif

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BIN_SRC=$(TOPDIR)/dl-bin/$(PKG_NAME)_$(PKG_BROADCOM_VERSION).tar.gz
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
    FILE_EXISTS=1
else
    FILE_EXISTS=0
endif

include $(INCLUDE_DIR)/package.mk

ifeq ($(FILE_EXISTS), 0)
export BRCM_DRIVER_BUILD_DIR:=../$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_FILE:=$(TOPDIR)/dl/$(PKG_SOURCE)
PKG_BUILD_FROM_SOURCE:=$(shell if [ -e $(PKG_SOURCE_FILE) ]; then echo y; fi)
IS_KERNEL_THUMB=$(shell (grep "^CONFIG_THUMB2_KERNEL=" $(LINUX_DIR)/.config | cut -d'=' -f 2))

ifeq ($(CONFIG_DEV_BROADCOM_ESDK_WLAN), y)
  PKG_BUILD_FROM_SOURCE:=y
endif 
BIN_PROFILE_BUILD:=$(PROFILE)
ifeq ($(CONFIG_TARGET_comcerto2000_nas),y)
	BIN_PROFILE_BUILD:=$(PROFILE)-nas
	ifneq ($(CONFIG_COMCERTO_PAGE_SIZE_64K),y)
		BIN_PROFILE_BUILD:=$(BIN_PROFILE_BUILD)-4k
	endif
else
	ifeq ($(CONFIG_TARGET_comcerto2000_hgw_nas),y)
		BIN_PROFILE_BUILD:=$(PROFILE)-hgw_nas
	endif
endif
ifeq ($(CONFIG_LIBC),"uClibc")
	BIN_PROFILE_BUILD:=$(BIN_PROFILE_BUILD)-uclibc
endif

include $(INCLUDE_DIR)/package.mk

export BROADCOMPATH=$(PKG_BUILD_DIR)/drivers

ifeq ($(ARCH),mips)
  HAL_TARGET:=mips-be-elf
endif
ifeq ($(ARCH),mipsel)
  HAL_TARGET:=mips-le-elf
endif
ifeq ($(ARCH),i386)
  HAL_TARGET:=i386-elf
endif
ifeq ($(ARCH),armeb)
  ifeq ($(word 1,$(subst ., ,$(GCC_VERSION))),4)
    HAL_TARGET:=armv6-be-elf
  else
    HAL_TARGET:=armv4-be-elf
  endif
endif
ifeq ($(ARCH),arm)
  ifeq ($(CONFIG_TARGET_comcerto2000), y)
    HAL_TARGET:=armv7-le-elf
  else
    ifeq ($(word 1,$(subst ., ,$(GCC_VERSION))),4)
      HAL_TARGET:=armv6-le-elf
    else
      HAL_TARGET:=armv4-le-elf
    endif
  endif
endif
ifeq ($(ARCH),powerpc)
  HAL_TARGET:=powerpc-be-elf
endif

BROADCOM_AUTOLOAD:= \
	wl

endif

################ OpenWRT definition ########################
define KernelPackage/broadcom_esdk_wlan
  CATEGORY:=Freescale Packages
  DEFAULT:=n
  DEPENDS:=+broadcom_esdk_wlan
  TITLE:=ESDK WLAN 802.11ac/n Wifi cards
  URL:=http://www.broadcom.com/
  MAINTAINER:=Freescale Technologies
  FILES:= $(PKG_BUILD_DIR)/wl.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,10,$(BROADCOM_AUTOLOAD))
endef

define KernelPackage/broadcom_esdk_wlan/description
  This package contains a driver for Broadcom cards.
endef

define Package/broadcom_esdk_wlan
  CATEGORY:=Freescale Packages
  DEFAULT:=n
  DEPENDS:=+libuci
  PKG_BUILD_DEPENDS:=pfe_ctrl
  TITLE:=ESDK WLAN 802.11ac/n Wifi cards libraries
  URL:=http://www.broadcom.com/
  MAINTAINER:=Freescale Technologies
endef

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
    echo "Fetching prebuilt binaries" &&\
    mkdir -p $(PKG_BUILD_DIR) &&\
    $(TAR) -xzf $(PKG_BIN_SRC) -C $(PKG_BUILD_DIR)
endef

else
ifeq ($(PKG_BUILD_FROM_SOURCE), y)
ifeq ($(CONFIG_DEV_BROADCOM_ESDK_WLAN), y)
ifeq ($(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH), "")

define Build/Prepare
	pushd "$(KERNEL_BUILD_DIR)" &&\
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITTREE) $(PKG_NAME)-$(PKG_VERSION) && \
	popd

	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
endef

else
define Build/Prepare
	pushd "$(KERNEL_BUILD_DIR)" &&\
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITTREE) $(PKG_NAME)-$(PKG_VERSION) && \
	pushd "$(PKG_NAME)-$(PKG_VERSION)" && \
	if [ -n "$$$$(git tag | grep -w $(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH) $(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH); \
	elif [ -n "$$$$(git branch -a | grep -w remotes\/origin\/$(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH) origin/$(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH); \
	else \
		echo "Invalid tag or branch name. \"$(CONFIG_DEV_BROADCOM_ESDK_WLAN_GITBRANCH)\""; \
		exit 1; \
	fi; \
	popd; \
	popd
	
	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
endef
endif
else
define Build/Prepare
	pushd "$(KERNEL_BUILD_DIR)" &&\
	$(TAR) xfz $(PKG_SOURCE_FILE) -C $(PKG_BUILD_DIR) && \
	popd
	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
endef
endif
endif
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	echo "Installing prebuilt binaries"
endef
else
define Build/Compile
	echo Actual compile start
	$(MAKE) -C ${PKG_BUILD_DIR}/build/ LINUX_DIR=${LINUX_DIR} TARGET_CROSS=${TARGET_CROSS} LINUX_KARCH=${LINUX_KARCH} PLT=${PLT}
	echo Actual compile end
	$(CP) $(PKG_BUILD_DIR)/main/components/router/target/lib/modules/$(LINUX_VERSION)/extra/wl/wl.$(LINUX_KMOD_SUFFIX) $(PKG_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/main/components/router/target/* $(PKG_BUILD_DIR)
	$(CP) ./files/$(BIN_PROFILE_BUILD)/* $(PKG_BUILD_DIR)
endef

endif


define Build/InstallDev
endef

define Build/UninstallDev
endef

define Package/broadcom_esdk_wlan/install
	echo broadcom_esdk_wlan-install start

	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/etc/config/wireless $(1)/etc/config/
	$(CP) $(PKG_BUILD_DIR)/etc/config/wireless_default $(1)/etc/config/
	$(CP) $(PKG_BUILD_DIR)/etc/config/cpprofile $(1)/etc/config/
	$(CP) $(PKG_BUILD_DIR)/etc/config/lobby $(1)/etc/config/
	$(CP) $(PKG_BUILD_DIR)/etc/config/wificonfig $(1)/etc/config/
	$(CP) $(PKG_BUILD_DIR)/etc/config/wlan_led $(1)/etc/config/
ifeq ($(CONFIG_TARGET_comcerto2000_hgw_c2krv340),y)
	$(CP) $(PKG_BUILD_DIR)/etc/config/wlan_led_rv160w $(1)/etc/config/
endif
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/etc/init.d/wifi_switch $(1)/etc/init.d/
	$(CP) $(PKG_BUILD_DIR)/etc/init.d/wlan_led $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/bin/eapd $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_cli $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_monitor $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_pbc $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_pin $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_ap_pin $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_pingen $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_pin_check $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/wps_daemon $(1)/bin/

	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/sbin/wifi_update $(1)/sbin/
	ln -sf /mnt/packages/wifi/nvram.txt $(1)/sbin/nvram.txt
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/usr/lib/libbcmcrypto.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/usr/lib/libbcm.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/usr/lib/libnvram.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/usr/lib/libshared.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/usr/lib/libbcmupnp.so $(1)/usr/lib/
	
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/usr/sbin/acs_cli $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/usr/sbin/acsd $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/usr/sbin/nas $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/usr/sbin/nvram $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/usr/sbin/wl $(1)/usr/sbin/
	
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_schedule $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_schedule_check $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_daemon $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_set $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wps_profile.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wps_get_profile.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/update_ssid_profile.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wps_nvram.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wps_reload.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_update_affinity $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_bttn_init.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/wifi_bttn_check.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/board_check.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/initial_ssid_add_rules.sh $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/usr/bin/initial_ssid_flush_rules.sh $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/lib/wifi
	$(CP) $(PKG_BUILD_DIR)/lib/wifi/* $(1)/lib/wifi/

	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)/extra/wl
	$(LN) /lib/modules/$(LINUX_VERSION)/wl.$(LINUX_KMOD_SUFFIX) $(1)/lib/modules/$(LINUX_VERSION)/extra/wl/wl.$(LINUX_KMOD_SUFFIX)
endef


$(eval $(call KernelPackage,broadcom_esdk_wlan))
$(eval $(call BuildPackage,broadcom_esdk_wlan))
