prefix = /usr/local
bindir = ${prefix}/bin

INSTALL = install
CC = gcc
#CPPFLAGS = -I/usr/include/libowfat 
#CPPFLAGS = -I. -D__USE_BSD=1
CPPFLAGS = -I.  -DPATH_MAX=4096
DEBUG := 1
ifeq ($(DEBUG),1)
CFLAGS = -g -ggdb -Wall -Wno-unused-variable -O0
else
CFLAGS =  -Os -fomit-frame-pointer -std=c99  
endif
#LIBS = -lowfat
EXEEXT =
HOST = $(shell $(CC) -dumpmachine |sed 's,.*-,,')


ifeq ($(HOST),mingw32)
				EXEEXT := .exe
				LIBS += -lshlwapi
endif
ifeq ($(HOST),msys)

				EXEEXT := .exe
				LIBS += -lshlwapi

endif


LIB_OBJ = buffer_close.o buffer_default.o buffer_dump.o buffer_feed.o buffer_flush.o buffer_free.o buffer_fromsa.o buffer_fromstr.o buffer_get.o buffer_get_new_token_sa.o buffer_get_new_token_sa_pred.o buffer_get_token.o buffer_get_token_pred.o buffer_get_token_sa.o buffer_get_token_sa_pred.o buffer_get_until.o buffer_getc.o buffer_getline.o buffer_getline_sa.o buffer_init.o buffer_mmapprivate.o buffer_mmapread.o buffer_mmapread_fd.o buffer_prefetch.o buffer_put.o buffer_putc.o buffer_putflush.o buffer_putm_internal.o buffer_putnlflush.o buffer_putnspace.o buffer_puts.o buffer_putsa.o buffer_putsflush.o buffer_putspace.o buffer_putulong.o buffer_skip_until.o buffer_stubborn.o buffer_stubborn2.o buffer_tosa.o buffer_truncfile.o byte_chr.o byte_copy.o byte_copyr.o byte_fill.o dir_close.o dir_open.o dir_read.o dir_type.o fmt_minus.o fmt_ulong.o fmt_ulong0.o mmap_private.o mmap_read.o mmap_read_fd.o open_read.o open_trunc.o shell_alloc.o shell_error.o shell_errorn.o shell_init.o shell_realloc.o str_len.o stralloc_append.o stralloc_cat.o stralloc_catb.o stralloc_catc.o stralloc_catlong0.o stralloc_cats.o stralloc_catulong0.o stralloc_copy.o stralloc_copyb.o stralloc_copys.o stralloc_diffs.o stralloc_free.o stralloc_init.o stralloc_insertb.o stralloc_move.o stralloc_nul.o stralloc_ready.o stralloc_readyplus.o stralloc_remove.o stralloc_trunc.o stralloc_write.o stralloc_zero.o mmap_read.o
PROGRAMS = list-r$(EXEEXT) count-depth$(EXEEXT) decode-ls-lR$(EXEEXT) torrent-progress$(EXEEXT)
all: $(PROGRAMS) 

decode-ls-lR.o: decode-ls-lR.c
decode-ls-lR$(EXEEXT): decode-ls-lR.o $(LIB_OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

count-depth.o: count-depth.c
count-depth$(EXEEXT): count-depth.o $(LIB_OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

list-r.o: list-r.c
list-r$(EXEEXT): list-r.o $(LIB_OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

torrent-progress.o: torrent-progress.c
torrent-progress$(EXEEXT): torrent-progress.o $(LIB_OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	$(RM) -f $(LIB_OBJ) list-r.o list-r$(EXEEXT)

install: $(PROGRAMS)
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 $(PROGRAMS) $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(bindir)
