# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pfe
PKG_VERSION:=devel.mainline
PKG_RELEASE:=2019-04-16_00_00__0530

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE).tar.gz
PKG_CAT:=zcat
PKG_SOURCE_FILE:=$(TOPDIR)/dl/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk

define Package/pfe
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=PFE Firmware
  URL:=http://www.freescale.com/
endef

define Package/pfe/description
 PFE firmware.
endef

define Package/pfe/config
	#source "$(SOURCE)/Config.in"
	#$(call Package/$(PKG_NAME)/override_source_path)
	menu "compile from sources"
		depends on PACKAGE_$(PKG_NAME) && (DEV_PFE)
	config $(PKG_NAME)_COMPILE_FROM_SOURCES
		depends on PACKAGE_$(PKG_NAME) && (DEV_PFE)
		bool "Compile from sources"
		default n
	config $(PKG_NAME)_TOOLCHAIN_PATH
		depends on $(PKG_NAME)_COMPILE_FROM_SOURCES
		string "Path to PFE toolchain"
		default "/proj/cpesw/toolchain/pfe/eSi-RISC"
	endmenu
endef

ifeq ($(CONFIG_TARGET_ls1012a),y)
       PFE_BIN_PATH:=ls1012a
       PFE_CLASS_FW_FILE:=obj_ls1012a/class/class_ls1012a.elf
       PFE_TMU_FW_FILE:=obj_c2000/tmu/tmu_c2000.elf
       PFE_UTIL_FW_FILE:=obj_ls1012a/util/util_ls1012a.elf
       PFE_COMPILE_FLAGS:=ls1012a
else
       PFE_BIN_PATH:=ls1024a
       PFE_CLASS_FW_FILE:=obj_c2000/class/class_c2000.elf
       PFE_TMU_FW_FILE:=obj_c2000/tmu/tmu_c2000.elf
       PFE_UTIL_FW_FILE:=obj_c2000/util/util_c2000.elf
       PFE_COMPILE_FLAGS:=c2000
endif

#If Development, we should download the version from our GIT instead of doing the generic process
ifeq ($(CONFIG_DEV_PFE), y)
ifeq ($(CONFIG_DEV_PFE_GITBRANCH), "")
define Build/Prepare
	pushd "$(KERNEL_BUILD_DIR)" && \
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_PFE_GITTREE) pfe-$(PKG_VERSION).$(PKG_RELEASE) && \
	popd
	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
endef
else
define Build/Prepare
	pushd "$(KERNEL_BUILD_DIR)" && \
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_PFE_GITTREE) pfe-$(PKG_VERSION).$(PKG_RELEASE) && \
	pushd "pfe-$(PKG_VERSION).$(PKG_RELEASE)" && \
	if [ -n "$$$$(git tag | grep -w $(CONFIG_DEV_PFE_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_PFE_GITBRANCH) $(CONFIG_DEV_PFE_GITBRANCH); \
	elif [ -n "$$$$(git branch -a | grep -w remotes\/origin\/$(CONFIG_DEV_PFE_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_PFE_GITBRANCH) origin/$(CONFIG_DEV_PFE_GITBRANCH); \
	else \
		echo "Invalid tag or branch name. \"$(CONFIG_DEV_PFE_GITBRANCH)\""; \
		exit 1; \
	fi; \
	popd; \
	popd
	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
endef
endif
endif
define Build/Configure
	if [ "$(CONFIG_TARGET_comcerto2000_hgw_c2krv340)" == ''y ]; then \
		$(SED) 's#^\(CFG_REALTEK_SWITCHTAG[^=]*=\).*$$$$#\1y#g' $(PKG_BUILD_DIR)/c2000/config.mk; \
	fi;
endef

ifeq ($(CONFIG_pfe_COMPILE_FROM_SOURCES), y)
define Build/Compile
	if [ $(CONFIG_pfe_TOOLCHAIN_PATH) == "" ]; then \
		make -C $(PKG_BUILD_DIR) $(PFE_COMPILE_TARGET); \
	else \
		make -C $(PKG_BUILD_DIR) $(PFE_COMPILE_TARGET) TOOL_PATH_GCC=$(CONFIG_pfe_TOOLCHAIN_PATH); \
	fi;

endef
define Package/pfe/install
	echo "pfe package"
	mkdir -p $(1)/lib/firmware
	$(CP) $(PKG_BUILD_DIR)/$(PFE_CLASS_FW_FILE) $(1)/lib/firmware/class_c2000.elf
	$(CP) $(PKG_BUILD_DIR)/$(PFE_TMU_FW_FILE) $(1)/lib/firmware/tmu_c2000.elf
	$(CP) $(PKG_BUILD_DIR)/$(PFE_UTIL_FW_FILE) $(1)/lib/firmware/util_c2000.elf
endef

else

define Build/Compile
endef

define Package/pfe/install
	echo "pfe package"
	$(CP) ./files/$(PFE_BIN_PATH)/lib $(1)
endef
endif


$(eval $(call BuildPackage,pfe))
