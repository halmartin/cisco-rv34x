include $(TOPDIR)/rules.mk

PKG_NAME:=perl-net-snmpkey
PKG_VERSION:=5.2.0
PKG_RELEASE:=1
PKG_MD5SUM:=0e717723f843ab22a93248833f3ebff7

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/D/DT/DTOWN/
PKG_SOURCE:=Net-SNMP-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Net-SNMP-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../perl/perlmod.mk

define Package/perl-net-snmpkey
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=Perl NET-SNMP snmpkey module
URL:=http://search.cpan.org/CPAN/authors/id/D/DT/DTOWN/
DEPENDS:=perl +perl-crypt-des +perl-digest-sha1 +perl-digest-hmac
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-net-snmpkey/install
	$(call perlmod/Install,$(1),Net auto/Net)
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/snmpkey $(1)/usr/bin/
endef


$(eval $(call BuildPackage,perl-net-snmpkey))

