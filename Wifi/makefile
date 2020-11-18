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



SOURCES = $(dir $(wildcard */makefile))

all:
	$(foreach SUBDIR,$(SOURCES),$(MAKE) -C $(SUBDIR);)
debug:
	$(foreach SUBDIR,$(SOURCES),$(MAKE) debug -C $(SUBDIR);)
clean:
	$(foreach SUBDIR,$(SOURCES),$(MAKE) clean -C $(SUBDIR);)
test-all:
	$(foreach SUBDIR,$(SOURCES),$(MAKE) -C $(SUBDIR) OBJDIR=$(NNDK_ROOT)/nb-build-test; rm -f $(NNDK_ROOT)/nb-build-test/release/src/*;rm -f $(NNDK_ROOT)/nb-build-test/release/netburner.a;)
test-debug:
	$(foreach SUBDIR,$(SOURCES),$(MAKE) debug -C $(SUBDIR) OBJDIR=$(NNDK_ROOT)/nb-build-test; rm -f $(NNDK_ROOT)/nb-build-test/debug/src/*;rm -f $(NNDK_ROOT)/nb-build-test/debug/dbnetburner.a;)
clean-test:
	rm -rf $(NNDK_ROOT)/nb-build-test
