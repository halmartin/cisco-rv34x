#
## Copyright (C) 2009-2011 OpenWrt.org
#
## This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# #
#
include $(TOPDIR)/rules.mk
PKG_NAME:=pam-krb5
PKG_VERSION:=4.7
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://archives.eyrie.org/software/kerberos
include $(INCLUDE_DIR)/package.mk

EXTRA_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5 -lldap
EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib -lldap
#EXTRA_LDFLAGS += -lkrb5 -lk5crypto -lresolv 
	
define Package/pam-krb5
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PAM module for Kerberos 5
  DEPENDS:= +krb5-libs +libpam +openldap-server
  URL:=http://http://www.eyrie.org/~eagle/software/pam-krb5/
  MENU:=1
endef

#define Package/pam-krb5/config
#	source "$(SOURCE)/Config.in"
#endef

define Package/pam-krb5/description
	PAM module for Kerbers 5
endef

EXTRA_CFLAGS=-I$(STAGING_DIR)/usr/include/krb5/ -lldap

define Build/Configure
    $(call Build/Configure/Default, \
		        --with-krb5=$(STAGING_DIR)/usr/ \
				--with-krb5-lib=$(STAGING_DIR)/usr/lib/ \
     	        )
endef

define Package/pam-krb5/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/security
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/pam_krb5.so $(1)/lib/security
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/ad.config $(1)/etc/config/ad
	$(INSTALL_DATA) ./files/krb5keytab $(1)/etc/
	chmod 777 $(1)/etc/krb5keytab
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ad.init $(1)/etc/init.d/ad

endef
ifeq ($(CONFIG_PAM-KRB5),y)
	$(CP) $(PKG_BUILD_DIR)/.libs/pam_krb5.so $(1)/lib/security
endif

$(eval $(call BuildPackage,pam-krb5))
