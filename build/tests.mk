
$(BUILDDIR)test_strlist.o: tests/test_strlist.c
$(BUILDDIR)test_strlist$(M64_)$(EXEEXT): $(BUILDDIR)test_strlist.o $(call add-library, strlist buffer stralloc str byte)
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) $(CFLAGS) $(CPPFLAGS) -Wl,-rpath=$(BUILDDIR:%/=%) -o $@ $^ $(LIBS) $(EXTRA_LIBS)

