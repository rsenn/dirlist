#include "../buffer.h"
#include "../json_internal.h"
#include "../stralloc.h"

static void
json_default_printer(jsonfmt* p, jsonval* v, int depth, int index, char q) {
  p->indent = "  ";
  p->newline = "\n";
  p->spacing = " ";
  p->quote[0] = '"';
};

void
json_tosa(jsonval val, stralloc* sa, json_print_fn* p) {
  buffer b;
  char buf[1024];

  if(p == NULL)
    p = &json_default_printer;

  buffer_init(&b, (buffer_op_sys*)(void*)&stralloc_write, -1, buf, sizeof(buf));
  b.cookie = sa;

  json_print(val, &b, p);
}
