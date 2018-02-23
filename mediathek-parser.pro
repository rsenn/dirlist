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

#mingw | mingw32 | mingw64 | msvc {
win32 | winn64 {
  DEFINES += USE_READDIR=0

} else {
   DEFINES += USE_READDIR=1
}

#win32|win64|msvc|mingw32:CONFIG(release, debug|release):  LIBS += -ladvapi32 -lws2_32
msvc: LIBS += advapi32.lib ws2_32.lib
mingw32|mingw64: LIBS += -ladvapi32 -lws2_32

mingw | mingw32 | mingw64 {
  QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}

!*msvc* {
  DEFINES += INLINE=inline

  #!*mingw*: DEFINES += USE_READDIR=1

  #QMAKE_CFLAGS_WARN_ON += -Wno-sign-compare -Wno-unused-parameter
}

INCLUDEPATH += . $$PWD/lib

HEADERS = \
 lib/buffer.h \
 lib/byte.h \
 lib/fmt.h \
 lib/scan.h \
 lib/stralloc.h \
 lib/str.h \
 lib/strlist.h \
 lib/uint32.h \
 lib/uint64.h


SOURCES = \
 mediathek-parser.c \
 lib/array/array_allocate.c \
 lib/array/array_length.c \
 lib/array/array_start.c \
 lib/array/array_trunc.c \
 lib/buffer/buffer_1.c \
 lib/buffer/buffer_2.c \
 lib/buffer/buffer_close.c \
 lib/buffer/buffer_feed.c \
 lib/buffer/buffer_flush.c \
 lib/buffer/buffer_getc.c \
 lib/buffer/buffer_getline_sa.c \
 lib/buffer/buffer_get_token_sa.c \
 lib/buffer/buffer_init.c \
 lib/buffer/buffer_mmapread.c \
 lib/buffer/buffer_munmap.c \
 lib/buffer/buffer_put.c \
 lib/buffer/buffer_putflush.c \
 lib/buffer/buffer_putlong.c \
 lib/buffer/buffer_putnlflush.c \
 lib/buffer/buffer_puts.c \
 lib/buffer/buffer_putm_internal.c \
 lib/buffer/buffer_putulong.c \
 lib/buffer/buffer_stubborn2.c \
 lib/buffer/buffer_stubborn.c \
 lib/byte/byte_chr.c \
 lib/byte/byte_copy.c \
 lib/byte/byte_rchr.c \
 lib/byte/byte_zero.c \
 lib/fmt/fmt_long.c \
 lib/fmt/fmt_ulong.c \
 lib/isleap.c \
 lib/mmap/mmap_read.c \
 lib/open/open_read.c \
 lib/stralloc/stralloc_append.c \
 lib/stralloc/stralloc_catb.c \
 lib/stralloc/stralloc_catc.c \
 lib/stralloc/stralloc_cats.c \
 lib/stralloc/stralloc_copyb.c \
 lib/stralloc/stralloc_copys.c \
 lib/stralloc/stralloc_free.c \
 lib/stralloc/stralloc_init.c \
 lib/stralloc/stralloc_nul.c \
 lib/stralloc/stralloc_ready.c \
 lib/stralloc/stralloc_readyplus.c \
 lib/stralloc/stralloc_zero.c \
 lib/str/str_chr.c \
 lib/str/str_diff.c \
 lib/str/str_len.c \
 lib/str/str_dup.c \
 lib/str/str_ptime.c \
 lib/time_table_spd.c \
 lib/umult64.c

include(deployment.pri)
qtcAddDeployment()

