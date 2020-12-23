#ifndef CPP_H
#define CPP_H

#include "buffer.h"

struct cpp;

struct cpp* cpp_new(void);
void cpp_free(struct cpp*);
void cpp_add_includedir(struct cpp* cpp, const char* includedir);
int cpp_add_define(struct cpp* cpp, const char* mdecl);
int cpp_run(struct cpp* cpp, buffer* in, buffer* out, const char* inname);

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#pragma RcB2 DEP "cpp.c"

#endif
