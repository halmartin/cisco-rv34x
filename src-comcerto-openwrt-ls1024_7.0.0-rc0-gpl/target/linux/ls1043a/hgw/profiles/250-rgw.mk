#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/lsrgw
	NAME:=RGW
endef

define Profile/lsrgw/Description
	Reference design for RGW using Freescale LS1043 SoC
endef

$(eval $(call Profile,lsrgw))

