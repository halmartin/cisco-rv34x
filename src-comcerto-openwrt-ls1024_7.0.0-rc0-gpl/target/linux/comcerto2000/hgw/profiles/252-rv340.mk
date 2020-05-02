# 2017-08-09: Ganesh Reddy <ganesh.redd@nxp.com>
# Add support for rv340.
#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/c2krv340
	NAME:=RV340
endef

define Profile/c2krv340/Description
	Reference design for RV34X using Mindspeed Comcerto-2000 SoC
endef

$(eval $(call Profile,c2krv340))

