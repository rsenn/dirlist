#include "../stralloc.h"
#include "../fmt.h"
#include "../json.h"

static void
json_tostring_printer(jsonfmt* p, jsonval* v, int depth, int index) {
  p->indent = "";
  p->newline = "";
  p->spacing = " ";
  p->quote = "\"";
};

const char*
json_tostring(jsonval val, stralloc* sa) {
  switch(val.type) {
    case JSON_UNDEFINED: {
      stralloc_cats(sa, "undefined");
      break;
    }
    case JSON_DOUBLE: {
      stralloc_readyplus(sa, 63);
      sa->len += fmt_double(&sa->s[sa->len], val.doublev, 0, 0);
      break;
    }
    case JSON_INT: {
      stralloc_readyplus(sa, FMT_LONG);
      sa->len += fmt_longlong(&sa->s[sa->len], val.intv);
      break;
    }
    case JSON_STRING: {
      stralloc_catb(sa, val.stringv.s, val.stringv.len);
      break;
    }
    default: {
      if(json_isnull(val))
        stralloc_cats(sa, "null");
      else
        json_tosa(val, sa, (void*)&json_tostring_printer);
      break;
    }
  }
  stralloc_nul(sa);
  return sa->s;
}

