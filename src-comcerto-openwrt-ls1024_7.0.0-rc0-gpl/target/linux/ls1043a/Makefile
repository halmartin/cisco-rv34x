#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

TARGET_ARCH=aarch64
ARCH:=aarch64
BOARD:=ls1043a
BOARDNAME:=LS1043
FEATURES:=fpu ramdisk jffs2 ubifs
CFLAGS:=-Os -pipe -fno-caller-saves
SUBTARGETS=hgw


define Target/Description
	Build firmeare images for Freescale Layerscape SoC
endef


KERNEL_PATCHVER:=4.1

ifeq ($(CONFIG_LINUX_3_19),y)
KERNEL_PATCHVER:=3.19
endif

ifeq ($(CONFIG_LINUX_4_1),y)
KERNEL_PATCHVER:=4.1
endif

DEVICE_TYPE:=developerboard

include $(INCLUDE_DIR)/target.mk

LINUX_PROFILE_CONFIG:=$(CURDIR)/$(SUBTARGET)/configs/$(PROFILE)

ifeq ($(CONFIG_DEV_KERNEL),y)
ifeq ($(CONFIG_DEV_KERNEL_GITBRANCH), "")
define Kernel/Prepare
	mkdir -p $(KERNEL_BUILD_DIR)
	pushd "$(KERNEL_BUILD_DIR)" && \
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_KERNEL_MODULENAME) linux-$(LINUX_VERSION) && \
	popd
endef
else
define Kernel/Prepare
	mkdir -p $(KERNEL_BUILD_DIR)
	pushd "$(KERNEL_BUILD_DIR)" && \
	git clone ssh://$(CONFIG_ACTIVEDEV_GITUSERNAME)@$(CONFIG_ACTIVEDEV_GITROOTPATH)/$(CONFIG_DEV_KERNEL_MODULENAME) linux-$(LINUX_VERSION) &&  \
	pushd "linux-$(LINUX_VERSION)" && \
	if [ -n "$$$$(git tag | grep -w $(CONFIG_DEV_KERNEL_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_KERNEL_GITBRANCH) $(CONFIG_DEV_KERNEL_GITBRANCH); \
	elif [ -n "$$$$(git branch -a | grep -w remotes\/origin\/$(CONFIG_DEV_KERNEL_GITBRANCH))" ]; then \
		git checkout -b $(CONFIG_DEV_KERNEL_GITBRANCH) origin/$(CONFIG_DEV_KERNEL_GITBRANCH); \
	else \
		echo "Invalid tag or branch name. \"$(CONFIG_DEV_KERNEL_GITBRANCH)\""; \
		exit 1; \
	fi; \
	popd; \
	popd
endef
endif
endif

define Kernel/SetInitramfs/PreConfigure
	grep -v -e INITRAMFS -e CONFIG_RD_ -e CONFIG_BLK_DEV_INITRD $(LINUX_DIR)/.config.old > $(LINUX_DIR)/.config
	echo 'CONFIG_BLK_DEV_INITRD=y' >> $(LINUX_DIR)/.config
	echo 'CONFIG_INITRAMFS_SOURCE=""' >> $(LINUX_DIR)/.config
endef

#KERNELNAME:=Image uImage dtbs
KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
