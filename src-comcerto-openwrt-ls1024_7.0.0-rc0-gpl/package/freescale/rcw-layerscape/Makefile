include $(TOPDIR)/rules.mk

PKG_NAME:=ls1xxx_rcw
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_CAT:=zcat


include $(INCLUDE_DIR)/package.mk

define Package/rcw-layerscape
  SECTION:= Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=rcw-layerscape
  DEPENDS:=@TARGET_ls1012a
  DEFAULT:=y if TARGET_ls1012a
  URL:=http://www.freescale.com/
endef

define Package/rcw-layerscape/Description
  rcw-layerscape
endef

ifeq ($(CONFIG_TARGET_ls1012a_hgw_lsrdb),y)
	BOARD_NAME:=rdb
endif

ifeq ($(CONFIG_TARGET_ls1012a_hgw_lsqds),y)
	BOARD_NAME:=qds
endif

ifeq ($(CONFIG_TARGET_ls1012a_nas_lsrdb),y)
	BOARD_NAME:=rdb
endif

ifeq ($(CONFIG_TARGET_ls1012a_nas_lsqds),y)
	BOARD_NAME:=qds
endif

ifeq ($(CONFIG_DEV_RCW_LAYERSCAPE),y)
ifneq ($(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH),"" )
define Build/Prepare
        pushd "$(BUILD_DIR)" && \
	git clone  ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_RCW_LAYERSCAPE_MODULENAME) $(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE) && \
	pushd "$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)" && \
	if [ -n "$$$$(git tag | grep -w $(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH) $(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH); \
	elif [ -n "$$$$(git branch -a | grep -w remotes\/origin\/$(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH) origin/$(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH); \
	else \
		echo "Invalid tag or branch name. \"$(CONFIG_DEV_RCW_LAYERSCAPE_GITBRANCH)\""; \
		exit 1; \
	fi; \
	popd; \
	popd
	if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR)  ./patches; \
	fi
endef
endif
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ls1012aqds
	$(MAKE) -C $(PKG_BUILD_DIR)/ls1012ardb
	mkdir -p $(BIN_DIR)/rcw
	mkdir -p $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)
	mkdir -p $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508
	$(CP) $(PKG_BUILD_DIR)/ls1012a$(BOARD_NAME)/R_SPNH_3508/*.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/
ifeq ($(CONFIG_TARGET_ls1012a_hgw_lsqds),y)
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_swap.bin 8
endif
ifeq ($(CONFIG_TARGET_ls1012a_nas_lqds),y)
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/rcw_800_swap.bin 8
endif
ifeq ($(CONFIG_TARGET_ls1012a_hgw_lsrdb),y)
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_default.bin $(BIN_DIR)/rcw/ls1012ardb/R_SPNH_3508/rcw_800_default_swap.bin 8
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_sdhc2.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_sdhc2_swap.bin 8
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_nfc.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_nfc_swap.bin 8
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_kw41.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_kw41_swap.bin 8
endif
ifeq ($(CONFIG_TARGET_ls1012a_nas_lsrdb),y)
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_default.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_default_swap.bin 8
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_sdhc2.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_sdhc2_swap.bin 8
	tclsh $(PKG_BUILD_DIR)/ls1012aqds/byte_swap.tcl $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_kw41.bin $(BIN_DIR)/rcw/ls1012a$(BOARD_NAME)/R_SPNH_3508/rcw_800_kw41_swap.bin 8
endif
endef


define Build/InstallDev
	echo InstallDev end
endef

define Build/UninstallDev
	echo UninstallDev end
endef

$(eval $(call BuildPackage,rcw-layerscape))
