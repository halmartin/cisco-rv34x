include $(TOPDIR)/rules.mk

PKG_NAME:=perl-crypt-rijndael
PKG_VERSION:=1.13
PKG_RELEASE:=1
PKG_MD5SUM:=2af117c9ab4052cec05cf6737c5f3f45

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/L/LE/LEONT/
PKG_SOURCE:=Crypt-Rijndael-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Crypt-Rijndael-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../perl/perlmod.mk

define Package/perl-crypt-rijndael
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=CBC compliant Rijndael encryption module
URL:=http://search.cpan.org/dist/Crypt-Rijndael/
DEPENDS:=perl
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-crypt-rijndael/install
        $(call perlmod/Install,$(1),Crypt auto/Crypt)
endef


$(eval $(call BuildPackage,perl-crypt-rijndael))

