include $(TOPDIR)/rules.mk

PKG_NAME:=perl-crypt-des
PKG_VERSION:=2.07
PKG_RELEASE:=1
PKG_MD5SUM:=e845d24cf383ca4a30a3380a2735feac

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/D/DP/DPARIS/
PKG_SOURCE:=Crypt-DES-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Crypt-DES-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../perl/perlmod.mk

define Package/perl-crypt-des
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=Perl DES encryption module
URL:=http://search.cpan.org/CPAN/authors/id/D/DP/DPARIS/
DEPENDS:=perl
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-crypt-des/install
        $(call perlmod/Install,$(1),Crypt auto/Crypt)
endef


$(eval $(call BuildPackage,perl-crypt-des))

