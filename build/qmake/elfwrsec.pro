TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = elfwrsec

INCLUDEPATH += .

HEADERS = ../../lib/buffer.h ../../lib/byte.h ../../lib/elf.h ../../lib/fmt.h ../../lib/mmap.h ../../lib/open.h ../../lib/str.h
SOURCES = ../../elfwrsec.c ../../lib/buffer/buffer_1.c ../../lib/buffer/buffer_flush.c ../../lib/buffer/buffer_put.c ../../lib/buffer/buffer_putflush.c ../../lib/buffer/buffer_putlong.c ../../lib/buffer/buffer_putnc.c ../../lib/buffer/buffer_putnlflush.c ../../lib/buffer/buffer_putnspace.c ../../lib/buffer/buffer_puts.c ../../lib/buffer/buffer_putspad.c ../../lib/buffer/buffer_putulong.c ../../lib/buffer/buffer_putxlong0.c ../../lib/buffer/buffer_stubborn.c ../../lib/byte/byte_copy.c ../../lib/byte/byte_fill.c ../../lib/elf/elf_get_value.c ../../lib/elf/elf_header.c ../../lib/elf/elf_shstrtab.c ../../lib/fmt/fmt_long.c ../../lib/fmt/fmt_ulong.c ../../lib/fmt/fmt_xlong.c ../../lib/mmap/mmap_shared.c ../../lib/mmap/mmap_unmap.c ../../lib/open/open_rw.c ../../lib/str/str_diff.c ../../lib/str/str_len.c
gcc: QMAKE_CFLAGS_WARN_ON += -Wno-sign-compare -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
