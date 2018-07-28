#ifndef LIB_JSON_H_
#define LIB_JSON_H_

#include <sys/types.h>
#include "buffer.h"
#include "hmap.h"
#include "uint64.h"
#include "slist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  JSON_NONE = 0,
  JSON_BOOL,
  JSON_INT,
  JSON_DOUBLE,
  JSON_STRING,
  JSON_LIST,
  JSON_DICT,
} jsondata;

typedef struct {
  jsondata type;
   union tag {
     char boolv:1;
     int64 intv;
     double doublev;
      char *stringv;
      slist* listv;
      HMAP_DB* dictv;
  };
} jsonnode;

typedef struct jsonreader {
  buffer* b;
  jsonnode* doc;
} jsonreader;


#define node_is_closing(n) ((n)->name[0] == '/')

typedef int(json_read_callback_fn)(jsonreader *r, jsondata id, stralloc *name,
                                  stralloc *value, HMAP_DB **attrs);

typedef int(json_predicate_fn)();


void        json_reader_init(jsonreader*, buffer*);


#ifdef __cplusplus
}
#endif
#endif /* LIB_JSON_H_ */