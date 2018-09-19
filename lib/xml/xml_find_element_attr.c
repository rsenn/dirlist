#include "../xml.h"

struct tag_attr_value {
  const char *t, *a, *v;
};

static int
xml_tag_attr_value_pred(xmlnode* node, const char* tag, const char* attr, const char* value) {

  if(node->type == XML_ELEMENT && str_equal(tag, node->name)) {
    const char* a = xml_get_attribute(node, attr);
    if(a && str_equal(value, a)) return 1;
  }
  return 0;
}

xmlnode*
xml_find_element_attr(xmlnode* node, const char* tag, const char* attr, const char* value) {
  return xml_find_pred_3(node, xml_tag_attr_value_pred, tag, attr, value);
}
