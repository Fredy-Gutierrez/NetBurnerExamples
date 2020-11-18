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

#This is a minimal make file.
#anything that starts with a # is a comment
#
#To generate the dependancies automatically 
#run "make depend"
#
#
#To clean up the directory 
#run "make clean"
#Your responsibilities as a programmer:
#
# Run make depend whenever:
#	You add files to the project
#	You change what files are included in a source file
#
# make clean whenever you change this makefile.
#
#Setup the project root name
#This will built NAME.s19 and save it as $(NNDK_ROOT)/bin/NAME.s19+
#This will built NAME_App.s19 and save it as $(NNDK_ROOT)/bin/NAME_App.s19+
NAME	= wav_player
CXXSRCS := \
	src/wav_data_srcs/sin440.cpp \
	src/wav_data_srcs/nbdemo_8k.cpp \
	src/wav_data_srcs/nbdemo_44k.cpp \
	src/main.cpp \
	src/wavPlayer.cpp \
	src/effs_time.cpp \
	src/FileSystemUtils.cpp \
	src/ramdrv_mcf.cpp \

XTRALIB = $(NNDK_ROOT)/platform/$(PLATFORM)/original/lib/libFatFile.a
DBXTRALIB = $(NNDK_ROOT)/platform/$(PLATFORM)/original/lib/libFatFile.a

src/wav_data_srcs/sin440.cpp: 
	unzip src/wav_data_srcs.zip -d src

src/wav_data_srcs/nbdemo_8k.cpp: 
	unzip src/wav_data_srcs.zip -d src

src/wav_data_srcs/nbdemo_44k.cpp:
	unzip src/wav_data_srcs.zip -d src

#Uncomment and modify these lines if you have C or S files.
#CSRCS := foo.c
#ASRCS := foo.s
# CREATEDTARGS := htmldata.cpp
#include the file that does all of the automagic work!
include $(NNDK_ROOT)/make/main.mak

