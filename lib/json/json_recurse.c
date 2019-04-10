#include "../hmap.h"
#include "../json.h"
#include "../slist.h"

static void
json_recurse_val(jsonval* val, void (*fn)(), void* arg, int depth) {
  fn(val, arg, depth);
  switch(val->type) {
    case JSON_UNDEFINED:
    case JSON_STRING:
    case JSON_DOUBLE:
    case JSON_BOOL:
    case JSON_INT: break;

    case JSON_OBJECT: {
      TUPLE* t;

      if(val->dictv && val->dictv->list_tuple) {
        hmap_foreach(val->dictv, t) { json_recurse_val(t->vals.val_custom, fn, arg, depth + 1); }
      }
      break;
    }
    case JSON_ARRAY: {
      slink* link;
      slist_foreach(val->listv, link) { json_recurse_val(slist_data(link), fn, arg, depth + 1); }
      break;
    }
  }
}

void
json_recurse(jsonval* val, void (*fn)(), void* arg) {
  json_recurse_val(val, fn, arg, 0);
}
