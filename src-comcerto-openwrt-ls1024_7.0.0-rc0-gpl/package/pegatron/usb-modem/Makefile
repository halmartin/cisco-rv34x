include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=usb-modem
PKG_VERSION=1.0.00.01
PKG_RELEASE:=1
PKG_TARGETS:=ipkg bin

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-modem
  SECTION:=Pegatron
  CATEGORY:=Pegatron Packages
  TITLE:=3G/4G USB Modem Driver
  DEPENDS:=+comgt +comgt-ncm +comgt-directip +uqmi +usb-modeswitch +libqmi +bwmgmt +usb-modem-mgmt
  KCONFIG:=\
	CONFIG_USB_ACM \
	CONFIG_USB_SERIAL \
	CONFIG_USB_SERIAL_OPTION \
	CONFIG_USB_SERIAL_WWAN \
	CONFIG_USB_SERIAL_QUALCOMM \
	CONFIG_USB_SERIAL_SIERRAWIRELESS \
	CONFIG_USB_WDM \
	CONFIG_USB_USBNET CONFIG_MII=y\
	CONFIG_USB_NET_QMI_WWAN \
	CONFIG_USB_SIERRA_NET \
	CONFIG_USB_NET_CDCETHER\
	CONFIG_USB_NET_RNDIS_HOST \
	CONFIG_USB_NET_CDC_NCM \
	CONFIG_USB_NET_HUAWEI_CDC_NCM
	
  FILES:= \
	$(LINUX_DIR)/drivers/usb/class/cdc-acm.ko \
	$(LINUX_DIR)/drivers/usb/class/cdc-wdm.ko \
	$(LINUX_DIR)/drivers/usb/serial/usbserial.ko \
	$(LINUX_DIR)/drivers/usb/serial/usb_wwan.ko \
	$(LINUX_DIR)/drivers/usb/serial/qcserial.ko \
	$(LINUX_DIR)/drivers/usb/serial/sierra.ko \
	$(LINUX_DIR)/drivers/usb/serial/option.ko \
	$(LINUX_DIR)/drivers/net/usb/usbnet.ko \
	$(LINUX_DIR)/drivers/net/usb/qmi_wwan.ko \
	$(LINUX_DIR)/drivers/net/usb/sierra_net.ko \
	$(LINUX_DIR)/drivers/net/usb/cdc_ether.ko \
	$(LINUX_DIR)/drivers/net/usb/rndis_host.ko \
	$(LINUX_DIR)/drivers/net/usb/cdc_ncm.ko \
	$(LINUX_DIR)/drivers/net/usb/huawei_cdc_ncm.ko

endef

define KernelPackage/usb-modem/description
  Kernel support for 3G/4G USB Modem Driver
endef

define Build/Prepare
	@mkdir -p $(PKG_BUILD_DIR)/files
endef

define Build/Compile
	$(foreach p,$(subst +,,$(DEPENDS)), \
		$(FIND) $(BUILD_DIR) -name $(p) -type d | grep /ipkg-$(PKGARCH)/$(p) | xargs -i $(TAR) -cC{} . | $(TAR) -xC$(PKG_BUILD_DIR)/files; \
	)
	rm -rf $(PKG_BUILD_DIR)/files/CONTROL
endef

define KernelPackage/usb-modem/install
	$(INSTALL_DIR) $(1)/etc/modules.d
	echo $(foreach file,$(FILES),$(basename $(notdir $(file)))) | tr ' ' '\012' > $(1)/etc/modules.d/60-usb-modem
	echo $(PKG_VERSION) >> $(1)/etc/usb-modem.ver
	$(CP) $(PKG_BUILD_DIR)/files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
	@[ "$$$$$$$$(dirname $(1))" = "$(BIN_DIR)" ] && (archive="$$$$$$$$(basename $(1))-$(PKG_VERSION).tar.gz"; $(RSTRIP) "$(1)" && cd "$(1)" && find ! -type d -printf '%P\n' | sort | grep -v '^\(install\.list\|fwsupport\)$$$$$$$$' >$(1)/install.list && tar --owner=0 --group=0 -zcf$$$$$$$${archive} -T$(1)/install.list fwsupport install.list; ls -A | grep -v "^$$$$$$$${archive}$$$$$$$$" | xargs rm -rf) || true
	@$(RM) $(1)/fwsupport	
endef

$(eval $(call KernelPackage,usb-modem))
