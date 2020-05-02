include $(TOPDIR)/rules.mk

PKG_NAME:=perl-digest-sha1
PKG_VERSION:=2.13
PKG_RELEASE:=1
PKG_MD5SUM:=bd22388f268434f2b24f64e28bf1aa35

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/G/GA/GAAS/
PKG_SOURCE:=Digest-SHA1-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Digest-SHA1-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../perl/perlmod.mk

define Package/perl-digest-sha1
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=Perl Digest SHA1 encryption module
URL:=http://search.cpan.org/CPAN/authors/id/G/GA/GAAS/
DEPENDS:=perl
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-digest-sha1/install
        $(call perlmod/Install,$(1),Digest auto/Digest)
endef


$(eval $(call BuildPackage,perl-digest-sha1))

