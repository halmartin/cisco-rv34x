include $(TOPDIR)/rules.mk

PKG_NAME:=ppa-generic
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_CAT:=zcat


include $(INCLUDE_DIR)/package.mk

define Package/ppa-layerscape
  SECTION:= Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=ppa-layerscape
  DEPENDS:=@TARGET_ls1012a
  URL:=http://www.freescale.com/
endef

define Package/ppa-layerscape/Description
  ppa-layerscape
endef

ifeq ($(CONFIG_DEV_PPA_LAYERSCAPE),y)
define Build/Prepare
        pushd "$(BUILD_DIR)" && \
	git clone  ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_PPA_LAYERSCAPE_MODULENAME) $(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE) && \
	pushd "$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)" && \
	if [ -n "$$$$(git tag | grep -w $(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH) $(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH); \
	elif [ -n "$$$$(git branch -a | grep -w remotes\/origin\/$(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH) origin/$(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH); \
	else \
		echo "Invalid tag or branch name. \"$(CONFIG_DEV_PPA_LAYERSCAPE_GITBRANCH)\""; \
		exit 1; \
	fi; \
	popd; \
	popd
	if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR)  ./patches; \
	fi
endef
endif

define Build/Compile
	make  -C $(PKG_BUILD_DIR)/ppa/soc-ls1012/platform-rdb/ CROSS_COMPILE=$(TARGET_CROSS) BUILD_BIN=$(STAGING_DIR_HOST)/bin rdb-fit
	mkdir -p $(BIN_DIR)/ppa
	$(CP) $(PKG_BUILD_DIR)/ppa/soc-ls1012/platform-rdb/build/obj/ppa.itb  $(BIN_DIR)/ppa/
	tclsh $(SCRIPT_DIR)/byte_swap.tcl $(BIN_DIR)/ppa/ppa.itb $(BIN_DIR)/ppa/ppa_swap.itb 8
	rm -rf $(BIN_DIR)/ppa/ppa.itb
endef


define Build/InstallDev
	echo InstallDev end
endef

define Build/UninstallDev
	echo UninstallDev end
endef

$(eval $(call BuildPackage,ppa-layerscape))
