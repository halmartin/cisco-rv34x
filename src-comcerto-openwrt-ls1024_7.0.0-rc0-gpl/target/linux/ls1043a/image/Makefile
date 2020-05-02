#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

BOARD_TYPE:=rdb
PLATFORM:=ls
SUBPLATFORM:=hgw
ifeq ($(CONFIG_TARGET_ls1043a_hgw),y)
	SUBPLATFORM:=hgw
endif
ifeq ($(CONFIG_TARGET_ls1043a_hgw_lsrdb),y)
	BOARD_TYPE:=rdb
endif
ifeq ($(CONFIG_TARGET_ls1043a_hgw_lsqds),y)
	BOARD_TYPE:=qds
endif

ifeq ($(CONFIG_TARGET_ls1043a_hgw_ls1012aqdsintpsr),y)
	BOARD_TYPE:=ls1012aqdsintpsr
endif
ifeq ($(CONFIG_TARGET_ls1043a_hgw_lsrgw),y)
	BOARD_TYPE:=rgw
endif

ifneq ($(CONFIG_TARGET_UBIFS_NAND_MIN_IO_SIZE),)
  UBIFS_NAND_OPTS += -U -q -m $(CONFIG_TARGET_UBIFS_NAND_MIN_IO_SIZE)
  UBINIZE_NAND_OPTS += -m $(CONFIG_TARGET_UBIFS_NAND_MIN_IO_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NAND_PHYSICAL_ERASE_BLOCK_SIZE),)
  UBINIZE_NAND_OPTS += -p $(CONFIG_TARGET_UBIFS_NAND_PHYSICAL_ERASE_BLOCK_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NAND_SUB_PAGE_SIZE),"")
  UBINIZE_NAND_OPTS += -s $(CONFIG_TARGET_UBIFS_NAND_SUB_PAGE_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NAND_LOGICAL_ERASE_BLOCK_SIZE),)
  UBIFS_NAND_OPTS += -e $(CONFIG_TARGET_UBIFS_NAND_LOGICAL_ERASE_BLOCK_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NAND_MAX_LEB_COUNT),)
  UBIFS_NAND_OPTS += -c $(CONFIG_TARGET_UBIFS_NAND_MAX_LEB_COUNT)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NAND_IMAGE_NAME),"")
  UBIFS_NAND_IMAGE_CMD = cp -f $(KDIR)/root.ubi $(CONFIG_TARGET_UBIFS_NAND_IMAGE_NAME)
endif
  UBIFS_NAND_OPTS += -x zlib

ifneq ($(CONFIG_TARGET_UBIFS_NOR_MIN_IO_SIZE),)
  UBIFS_NOR_OPTS += -U -q -m $(CONFIG_TARGET_UBIFS_NOR_MIN_IO_SIZE)
  UBINIZE_NOR_OPTS += -m $(CONFIG_TARGET_UBIFS_NOR_MIN_IO_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NOR_PHYSICAL_ERASE_BLOCK_SIZE),)
  UBINIZE_NOR_OPTS += -p $(CONFIG_TARGET_UBIFS_NOR_PHYSICAL_ERASE_BLOCK_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NOR_LOGICAL_ERASE_BLOCK_SIZE),)
  UBIFS_NOR_OPTS += -e $(CONFIG_TARGET_UBIFS_NOR_LOGICAL_ERASE_BLOCK_SIZE)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NOR_MAX_LEB_COUNT),)
  UBIFS_NOR_OPTS += -c $(CONFIG_TARGET_UBIFS_NOR_MAX_LEB_COUNT)
endif
ifneq ($(CONFIG_TARGET_UBIFS_NOR_IMAGE_NAME),"")
  UBIFS_NOR_IMAGE_CMD = cp -f $(KDIR)/root.ubi $(CONFIG_TARGET_UBIFS_NOR_IMAGE_NAME)
endif
  UBIFS_NOR_OPTS += -x zlib

ifeq ($(CONFIG_LINUX_3_19),y)
KERNEL_PATCHVER:=3.19
endif

ifeq ($(CONFIG_LINUX_4_1),y)
KERNEL_PATCHVER:=4.1
endif
KERNEL_LOAD_ADDR:=0x80080000
KERNEL_ENTRYPOINT_ADDR:=0x80080000

define Image/Prepare
	cp $(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/Image $(KDIR)/Image
endef

define Build/Clean
	$(MAKE) -C boot-wrapper clean
endef

define Image/Build/QemuVirt
	$(CP) $(KDIR)/Image $(BIN_DIR)/openwrt-$(BOARD)-qemu-virt.Image
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/openwrt-$(BOARD)-qemu-virt-initramfs.Image
endif
endef



ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
define Image/mkfs/ubifs
	$(CP) ./ubinize_nor.cfg $(KDIR)
	$(STAGING_DIR_HOST)/bin/mkfs.ubifs $(UBIFS_NOR_OPTS) -o $(KDIR)/root.ubifs_nor -d $(TARGET_DIR)
	(cd $(KDIR); \
	$(STAGING_DIR_HOST)/bin/ubinize $(UBINIZE_NOR_OPTS) -o $(KDIR)/root.ubi_nor ubinize_nor.cfg)
	$(call Image/Build,ubi_nor)
	$(UBIFS_NOR_IMAGE_CMD)
	$(CP) ./ubinize_nand.cfg $(KDIR)
	$(STAGING_DIR_HOST)/bin/mkfs.ubifs $(UBIFS_NAND_OPTS) -o $(KDIR)/root.ubifs_nand -d $(TARGET_DIR)
	(cd $(KDIR); \
	$(STAGING_DIR_HOST)/bin/ubinize $(UBINIZE_NAND_OPTS) -o $(KDIR)/root.ubi_nand ubinize_nand.cfg)
	$(call Image/Build,ubi_nand)
	$(UBIFS_NAND_IMAGE_CMD)
endef
endif

define Image/Build/uImage
        $(STAGING_DIR_HOST)/bin/mkimage -A arm64 -O linux -T kernel -C none -a $(KERNEL_LOAD_ADDR) -e $(KERNEL_ENTRYPOINT_ADDR) -n 'Linux $(KERNEL_PATCHVER)' -d $(KDIR)/Image $(BIN_DIR)/openwrt-$(BOARD)-$(SUBPLATFORM)-${PROFILE}-uImage
endef

define Image/Build/dtb
	$(CP) $(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/dts/freescale/fsl-$(BOARD)-$(BOARD_TYPE).dtb $(BIN_DIR)/openwrt-$(BOARD)-$(SUBPLATFORM)-${PROFILE}.dtb
endef

define Image/Build/VexpressFoundation
	$(CP) $(KDIR)/linux-system.axf $(BIN_DIR)/openwrt-$(BOARD)-vexpress-foundation.axf
endef

define Image/BuildKernel
	$(call Image/Build/QemuVirt)
	$(call Image/Build/uImage)
	$(call Image/Build/dtb)
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

ifeq ($(CONFIG_ITB_IMAGE),y)
define Image/Build/itb
	gzip -9 -c $(KDIR)/Image > $(KDIR)/Image.gz
	$(CP) $(BIN_DIR)/openwrt-$(BOARD)-$(SUBPLATFORM)-rootfs.cpio.gz $(KDIR)/
	$(CP) $(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/dts/freescale/fsl-$(BOARD)-$(BOARD_TYPE).dtb $(KDIR)/
	$(CP) $(LINUX_DIR)/kernel-$(BOARD)-$(BOARD_TYPE).its $(KDIR)/
	$(SED) 's/arch\/arm64\/boot\/Image.gz/Image.gz/' $(KDIR)/kernel-$(BOARD)-$(BOARD_TYPE).its
	$(SED) 's/arch\/arm64\/boot\/dts\/freescale\/fsl-$(BOARD)-$(BOARD_TYPE).dtb/fsl-$(BOARD)-$(BOARD_TYPE).dtb/' $(KDIR)/kernel-$(BOARD)-$(BOARD_TYPE).its
	$(SED) 's/fsl-image-core-${BOARD}${BOARD_TYPE}.ext2.gz/openwrt-$(BOARD)-$(SUBPLATFORM)-rootfs.cpio.gz/' $(KDIR)/kernel-$(BOARD)-$(BOARD_TYPE).its
	pushd $(KDIR) &&\
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) &&\
	mkimage -f kernel-$(BOARD)-$(BOARD_TYPE).its openwrt-$(BOARD)-$(SUBPLATFORM)-${PROFILE}.itb &&\
	$(CP) openwrt-$(BOARD)-$(SUBPLATFORM)-${PROFILE}.itb $(BIN_DIR)/ &&\
	popd
endef
else
define Image/Build/itb
	true
endef
endif

define Image/Build
	$(call Image/Build/$(1))
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-root.$(1) bs=128k conv=sync
endef

$(eval $(call BuildImage))
