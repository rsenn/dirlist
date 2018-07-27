TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

*msvc* {
  DEFINES += INLINE=__inline
  DEFINES += _CRT_NONSTDC_NO_DEPRECATE=1
  DEFINES += _CRT_SECURE_NO_WARNINGS=1

  QMAKE_CFLAGS_WARN_ON = -W3
}

!*msvc* {
  DEFINES += INLINE=inline

  QMAKE_CFLAGS_WARN_ON += -Wno-sign-compare
}

INCLUDEPATH += . $$PWD/lib

HEADERS = \
  lib/buffer.h \
  lib/byte.h \
  lib/fmt.h \
  lib/mmap.h \
  lib/open.h

SOURCES = lib/buffer/buffer_0.c lib/buffer/buffer_0small.c lib/buffer/buffer_1.c lib/buffer/buffer_1small.c lib/buffer/buffer_2.c lib/buffer/buffer_close.c lib/buffer/buffer_default.c lib/buffer/buffer_dummyread.c lib/buffer/buffer_dummyreadmmap.c lib/buffer/buffer_dump.c lib/buffer/buffer_feed.c lib/buffer/buffer_flush.c lib/buffer/buffer_free.c lib/buffer/buffer_fromarray.c lib/buffer/buffer_frombuf.c lib/buffer/buffer_fromsa.c lib/buffer/buffer_fromstr.c lib/buffer/buffer_get.c lib/buffer/buffer_getc.c lib/buffer/buffer_getline.c lib/buffer/buffer_getline_sa.c lib/buffer/buffer_getn.c lib/buffer/buffer_getnewline_sa.c lib/buffer/buffer_get_new_token_sa.c lib/buffer/buffer_get_new_token_sa_pred.c lib/buffer/buffer_get_token.c lib/buffer/buffer_get_token_pred.c lib/buffer/buffer_get_token_sa.c lib/buffer/buffer_get_token_sa_pred.c lib/buffer/buffer_get_until.c lib/buffer/buffer_init.c lib/buffer/buffer_init_free.c lib/buffer/buffer_mmapprivate.c lib/buffer/buffer_mmapread.c lib/buffer/buffer_mmapread_fd.c lib/buffer/buffer_munmap.c lib/buffer/buffer_peek.c lib/buffer/buffer_prefetch.c lib/buffer/buffer_put.c lib/buffer/buffer_put8long.c lib/buffer/buffer_putalign.c lib/buffer/buffer_putc.c lib/buffer/buffer_puterror.c lib/buffer/buffer_puterror2.c lib/buffer/buffer_putflush.c lib/buffer/buffer_putlong.c lib/buffer/buffer_putlonglong.c lib/buffer/buffer_putm_internal.c lib/buffer/buffer_putm_internal_flush.c lib/buffer/buffer_putnlflush.c lib/buffer/buffer_putnspace.c lib/buffer/buffer_puts.c lib/buffer/buffer_putsa.c lib/buffer/buffer_putsaflush.c lib/buffer/buffer_putsalign.c lib/buffer/buffer_putsflush.c lib/buffer/buffer_putspace.c lib/buffer/buffer_putuint64.c lib/buffer/buffer_putulong.c lib/buffer/buffer_putulonglong.c lib/buffer/buffer_putxlong.c lib/buffer/buffer_read_fd.c lib/buffer/buffer_seek.c lib/buffer/buffer_skip_until.c lib/buffer/buffer_stubborn.c lib/buffer/buffer_stubborn2.c lib/buffer/buffer_tosa.c lib/buffer/buffer_truncfile.c lib/byte/byte_chr.c lib/byte/byte_copy.c lib/byte/byte_copyr.c lib/byte/byte_diff.c lib/byte/byte_fill.c lib/byte/byte_rchr.c lib/byte/byte_zero.c lib/fmt/fmt_8long.c lib/fmt/fmt_8longlong.c lib/fmt/fmt_asn1derlength.c lib/fmt/fmt_asn1dertag.c lib/fmt/fmt_double.c lib/fmt/fmt_escapecharc.c lib/fmt/fmt_escapecharhtml.c lib/fmt/fmt_escapecharquotedprintable.c lib/fmt/fmt_escapecharquotedprintableutf8.c lib/fmt/fmt_escapecharxml.c lib/fmt/fmt_fill.c lib/fmt/fmt_httpdate.c lib/fmt/fmt_human.c lib/fmt/fmt_humank.c lib/fmt/fmt_iso8601.c lib/fmt/fmt_long.c lib/fmt/fmt_longlong.c lib/fmt/fmt_minus.c lib/fmt/fmt_pad.c lib/fmt/fmt_plusminus.c lib/fmt/fmt_str.c lib/fmt/fmt_strm_internal.c lib/fmt/fmt_strn.c lib/fmt/fmt_tohex.c lib/fmt/fmt_uint64.c lib/fmt/fmt_ulong.c lib/fmt/fmt_ulong0.c lib/fmt/fmt_ulonglong.c lib/fmt/fmt_utf8.c lib/fmt/fmt_xlong.c lib/fmt/fmt_xlonglong.c lib/fmt/fmt_xmlescape.c lib/mmap/mmap_map.c lib/mmap/mmap_private.c lib/mmap/mmap_read.c lib/mmap/mmap_read_fd.c lib/mmap/mmap_unmap.c lib/open/open_append.c lib/open/open_read.c lib/open/open_rw.c lib/open/open_trunc.c lib/str/str_basename.c lib/str/str_case_diff.c lib/str/str_case_diffn.c lib/str/str_case_equal.c lib/str/str_case_start.c lib/str/str_cat.c lib/str/str_chr.c lib/str/str_copy.c lib/str/str_copyb.c lib/str/str_decamelize.c lib/str/str_diff.c lib/str/str_diffn.c lib/str/str_dup.c lib/str/str_equal.c lib/str/str_find.c lib/str/str_findb.c lib/str/str_istr.c lib/str/str_len.c lib/str/str_lower.c lib/str/str_ptime.c lib/str/str_rchr.c lib/str/str_start.c lib/str/str_tok.c torrent-progress.c

include(deployment.pri)
qtcAddDeployment()

