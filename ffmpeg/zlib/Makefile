LIBNAME=libffdshow_zlib.a

SRCS_C=adler32.c compress.c crc32.c deflate.c trees.c zutil.c

include ../makefile.inc

all:    $(LIBNAME)

clean:	
	$(RM) $(TEMPFILES) $(LIBNAME)

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
