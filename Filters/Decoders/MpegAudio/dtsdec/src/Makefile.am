AM_CFLAGS = $(DTSDEC_CFLAGS)

bin_PROGRAMS = dtsdec extract_dts
dtsdec_SOURCES = dtsdec.c getopt.c gettimeofday.c
dtsdec_LDADD = $(top_builddir)/libdts/libdts.la \
	       $(top_builddir)/libao/libao.a -lm $(LIBAO_LIBS)
extract_dts_SOURCES = extract_dts.c getopt.c

man_MANS = dtsdec.1 extract_dts.1

EXTRA_DIST = configure.incl getopt.h gettimeofday.h $(man_MANS)
