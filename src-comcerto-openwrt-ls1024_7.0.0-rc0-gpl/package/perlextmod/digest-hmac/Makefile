include $(TOPDIR)/rules.mk

PKG_NAME:=perl-digest-hmac
PKG_VERSION:=1.03
PKG_RELEASE:=1
PKG_MD5SUM:=e6a5d6f552da16eacb5157ea4369ff9d

PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/G/GA/GAAS/
PKG_SOURCE:=Digest-HMAC-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Digest-HMAC-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../perl/perlmod.mk

define Package/perl-digest-hmac
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
TITLE:=Perl Digest Digest HMAC encryption module
URL:=http://search.cpan.org/CPAN/authors/id/G/GA/GAAS/
DEPENDS:=perl +perl-digest-sha1
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-digest-hmac/install
        $(call perlmod/Install,$(1),Digest auto/Digest)
endef


$(eval $(call BuildPackage,perl-digest-hmac))

