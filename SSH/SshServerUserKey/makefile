# Revision: 3.2.0 

###############################################################################
# Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
#
#    Permission is hereby granted to purchasers of NetBurner Hardware to use or
#    modify this computer program for any use as long as the resultant program
#    is only executed on NetBurner provided hardware.
#
#    No other rights to use this program or its derivatives in part or in
#    whole are granted.
#
#    It may be possible to license this or other NetBurner software for use on
#    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
#
#    NetBurner makes no representation or warranties with respect to the
#    performance of this computer program, and specifically disclaims any
#    responsibility for any damages, special or consequential, connected with
#    the use of this program.
#
# NetBurner
# 16855 W Bernardo Dr #260
# San Diego, CA 92127
# www.netburner.com
###############################################################################


# This makefile will default to the 2MB AM29LV160B flash chip
# Change to a different flash chip file in the CSRCS: line to use a different chip.
#PLATFORM=SB70LC
#DBXTRALIB :=$(NNDK_ROOT)/lib/DBsshLibrary.a $(NBROOT)/lib/DBcryptolib.a $(NBROOT)/lib/debugLibrary.a $(NBROOT)/lib/StdFFile.a
#XTRALIB :=$(NNDK_ROOT)/lib/sshLibrary.a $(NBROOT)/lib/cryptolib.a $(NBROOT)/lib/debugLibrary.a $(NBROOT)/lib/StdFFile.a
#COMPCODEFLAGS = 0xffc08000 0xFFD80000

#This will built NAME.x and save it as $(NNDK_ROOT)/bin/NAME.x
NAME	= SshUserKey
SUPPORTED_PLATFORMS = MODM7AE70 MOD5441X

CPP_SRC := src/main.cpp src/web.cpp src/nvsettings.cpp src/sshuser.cpp


CREATEDTARGS += src/htmldata.cpp
CPP_SRC += src/htmldata.cpp
src/htmldata.cpp : $(wildcard html/*.*)
	comphtml html -osrc/htmldata.cpp
	
XTRALIB = $(NNDK_ROOT)/platform/$(PLATFORM)/original/lib/libStdFFile.a
DBXTRALIB = $(NNDK_ROOT)/platform/$(PLATFORM)/original/lib/libStdFFile.a

# To clean just the common files, run "make clean-common"
include $(NNDK_ROOT)/examples/_common/EFFS/STD/common.mak

include $(NNDK_ROOT)/libraries/ssh/library.mak

include $(NNDK_ROOT)/make/boilerplate.mk
