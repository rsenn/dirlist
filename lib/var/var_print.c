#include "../str.h"
#include "../var.h"
#include "../buffer.h"
#include <assert.h>

/* print a variable, suitable for re-input
 * ----------------------------------------------------------------------- */
void
var_print(struct var* var, int flags) {
  if(flags & V_EXPORT) buffer_puts(buffer_1, "export ");

  /* output variable name */
  buffer_put(buffer_1, var->sa.s, var->len);

  /* if the variable wasn't unset we display it */
  if(!(var->flags & V_UNSET)) {
    unsigned long i;

    buffer_puts(buffer_1, "=\"");

    for(i = var->offset; i < var->sa.len; i++) {
      /* escape characters that must be escaped in double-quotation mode */
      if(parse_isdesc(var->sa.s[i])) buffer_puts(buffer_1, "\\");

      buffer_PUTC(buffer_1, var->sa.s[i]);
    }

    buffer_puts(buffer_1, "\"");
  }

  buffer_putnlflush(buffer_1);
}
