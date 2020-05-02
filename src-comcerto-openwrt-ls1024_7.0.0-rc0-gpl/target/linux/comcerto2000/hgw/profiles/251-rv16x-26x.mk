# 2017-02-06: harry.lin <harry.lin@deltaww.com>
# Add support for rv16x-rv26x.
#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/c2krv16x-26x
	NAME:=RV16X-26X
endef

define Profile/c2krv16x-26x/Description
	Reference design for RV16X and RV26X using Mindspeed Comcerto-2000 SoC
endef

$(eval $(call Profile,c2krv16x-26x))

