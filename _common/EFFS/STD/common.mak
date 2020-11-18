# The STD EFFS common utility headers are here
# NBINCLUDE += -I"$(NNDK_ROOT)/examples/_common/EFFS/STD"

# Source files
CPP_SRC		+= \
				src/FileSystemUtils.cpp \
				src/effsStdFlashDrv.cpp \
				src/effs_time.cpp \
				src/fs_main.cpp \
				src/ftp_fs.cpp \
				src/ramdrv_mcf.cpp \
				src/effs_std.cpp

COMMON_FILES := \
				src/FileSystemUtils.cpp \
				src/effsStdFlashDrv.cpp \
				src/effs_time.cpp \
				src/effs_std.cpp \
				src/fs_main.cpp \
				src/ftp_fs.cpp \
				src/http_f.h \
				src/FileSystemUtils.h \
				src/effs_std.h \
				src/effs_time.h \
				src/fs_main.h \
				src/ftp_fs.h \
				src/ramdrv_mcf.cpp \
				src/flashChip/AM29LV160B.h \
				src/flashChip/AT49BV163D.h \
				src/flashChip/MCF5282Flash.h \
				src/flashChip/MX29GL256F.h \
				src/flashChip/S29GL032.h \
				src/flashChip/SAME70Q21.h \
				src/flashChip/SST39VF040.h \

all: $(COMMON_FILES)

debug: $(COMMON_FILES)

$(COMMON_FILES):
	@mkdir -p src/flashChip
	cp $(NNDK_ROOT)/examples/_common/EFFS/STD/$@ $@

clean-common:
	rm -f $(COMMON_FILES)
	rmdir src/flashChip

