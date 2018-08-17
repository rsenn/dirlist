TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += link_pkgconfig
#PKGCONFIG += libxml-2.0

TARGET = xmltest2

INCLUDEPATH += . $$PWD/lib

SOURCES = lib/array/array_allocate.c lib/array/array_length.c lib/array/array_start.c lib/buffer/buffer_1.c lib/buffer/buffer_2.c lib/buffer/buffer_close.c lib/buffer/buffer_dummyread.c lib/buffer/buffer_feed.c lib/buffer/buffer_flush.c lib/buffer/buffer_gettok_sa.c lib/buffer/buffer_mmapprivate.c lib/buffer/buffer_munmap.c lib/buffer/buffer_peek.c lib/buffer/buffer_peekc.c lib/buffer/buffer_prefetch.c lib/buffer/buffer_put.c lib/buffer/buffer_putc.c lib/buffer/buffer_putdouble.c lib/buffer/buffer_putflush.c lib/buffer/buffer_putlong.c lib/buffer/buffer_putlonglong.c lib/buffer/buffer_putm_internal.c lib/buffer/buffer_putnlflush.c lib/buffer/buffer_putnspace.c lib/buffer/buffer_putptr.c lib/buffer/buffer_putsa.c lib/buffer/buffer_puts.c lib/buffer/buffer_putulong.c lib/buffer/buffer_putulonglong.c lib/buffer/buffer_skipc.c lib/buffer/buffer_skip_pred.c lib/buffer/buffer_skipspace.c lib/buffer/buffer_skip_until.c lib/buffer/buffer_stubborn2.c lib/buffer/buffer_stubborn.c lib/byte/byte_chr.c lib/byte/byte_copy.c lib/byte/byte_copyr.c lib/byte/byte_diff.c lib/byte/byte_equal.c lib/byte/byte_fill.c lib/byte/byte_scan.c lib/byte/byte_zero.c lib/fmt/fmt_double.c lib/fmt/fmt_escapecharc.c lib/fmt/fmt_long.c lib/fmt/fmt_longlong.c lib/fmt/fmt_repeat.c lib/fmt/fmt_ulong.c lib/fmt/fmt_ulonglong.c lib/fmt/fmt_xlonglong.c lib/hmap/hmap_add.c lib/hmap/hmap_destroy.c lib/hmap/hmap_free_data.c lib/hmap/hmap_init.c lib/hmap/hmap_search.c lib/hmap/hmap_set.c lib/hmap/hmap_truncate.c lib/mmap/mmap_private.c lib/open/open_read.c lib/scan/scan_fromhex.c lib/scan/scan_ulong.c lib/scan/scan_ulongn.c lib/scan/scan_xlong.c lib/scan/scan_xmlescape.c lib/stralloc/stralloc_append.c lib/stralloc/stralloc_catb.c lib/stralloc/stralloc_catc.c lib/stralloc/stralloc_cats.c lib/stralloc/stralloc_fmt_call.c lib/stralloc/stralloc_fmt_pred.c lib/stralloc/stralloc_free.c lib/stralloc/stralloc_init.c lib/stralloc/stralloc_insertb.c lib/stralloc/stralloc_nul.c lib/stralloc/stralloc_ready.c lib/stralloc/stralloc_readyplus.c lib/stralloc/stralloc_subst.c lib/stralloc/stralloc_trunc.c lib/stralloc/stralloc_zero.c lib/strlist/strlist_contains.c lib/strlist/strlist_froms.c lib/strlist/strlist_pushb.c lib/str/str_chr.c lib/str/str_diff.c lib/str/str_diffn.c lib/str/str_len.c lib/umult64.c lib/xml/xml_debug.c lib/xml/xml_find_all_pred.c lib/xml/xml_find_predicates.c lib/xml/xml_free.c lib/xml/xml_get_attribute.c lib/xml/xml_has_attribute.c lib/xml/xml_newnode.c lib/xml/xml_path.c lib/xml/xml_print_attributes.c lib/xml/xml_print_nodeset.c lib/xml/xml_read_callback.c lib/xml/xml_reader_init.c lib/xml/xml_read_tree.c xmltest2.c


