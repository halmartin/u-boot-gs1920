#!/bin/sh

#CROSS_COMPILE=~/toolchain/rsdk-1.3.6/linux/uclibc/bin/rsdk-linux- make $*
#CROSS_COMPILE=/rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-110915/bin/rsdk-linux- make $*
export STAGING_DIR=~/projects/openwrt/gs1920-24hp/openwrt/staging_dir/
CROSS_COMPILE=$STAGING_DIR/toolchain-mips_24kc_gcc-12.2.0_musl/bin/mips-openwrt-linux-musl- make $*
