AM_CFLAGS = $(OPT_CFLAGS) $(LIBDTS_CFLAGS)

lib_LTLIBRARIES = libdts.la

libdts_la_SOURCES = bitstream.c parse.c downmix.c
libdts_la_LIBADD = $(LIBDTS_LIBS) -lm
libdts_la_LDFLAGS = -no-undefined

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdts.pc

EXTRA_DIST = configure.incl dts_internal.h bitstream.h tables.h tables_quantization.h tables_huffman.h tables_fir.h tables_adpcm.h
