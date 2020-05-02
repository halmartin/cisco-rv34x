include $(TOPDIR)/rules.mk

PKG_NAME:=usdpaa
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_CAT:=zcat
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/usdpaa
  SECTION:= Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=usdpaa libraries
  DEPENDS:=@TARGET_ls1043a
  DEFAULT:=y if TARGET_ls1043a
  URL:=http://www.freescale.com/
endef

define Package/usdpaa/Description
  usdpaa library
endef

#If Development, we should downlaod the version from our GIT instead of doing the generic process
ifeq ($(CONFIG_DEV_USDPAA), y)

ifeq ($(CONFIG_DEV_USDPAA_GITBRANCH), "")
define Build/Prepare
	pushd "$(BUILD_DIR)" && \
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_USDPAA_GITTREE) $(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE) && \
	popd
	@if [ -d ./patches -a "$$$$(ls ./patches | wc -l)" -gt 0 ]; then \
		$(KPATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
endef

else
define Build/Prepare
	pushd "$(BUILD_DIR)" && \
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_USDPAA_GITTREE) $(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE) && \
	pushd "$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)" && \
	if [ -n "$$$$(git tag | grep -w $(CONFIG_DEV_USDPAA_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_USDPAA_GITBRANCH) $(CONFIG_DEV_USDPAA_GITBRANCH); \
	elif [ -n "$$$$(git branch -a | grep -w remotes\/origin\/$(CONFIG_DEV_USDPAA_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_USDPAA_GITBRANCH) origin/$(CONFIG_DEV_USDPAA_GITBRANCH); \
	else \
		echo "Invalid tag or branch name. \"$(CONFIG_DEV_USDPAA_GITBRANCH)\""; \
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


TARGET_CFLAGS += -I $(PKG_BUILD_DIR)/include -DCONFIG_ARM64
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		GCC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/usdpaa
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/usdpaa
endef

$(eval $(call BuildPackage,usdpaa))
