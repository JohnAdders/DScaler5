LIBNAME=libffdshow_png.a

SRCS_C=png.c pngwrite.c pngwutil.c

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
