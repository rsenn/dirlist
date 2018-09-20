#include "../buffer.h"
#include "../strlist.h"
#include "../xml.h"

int xml_has_attr(xmlnode* node, void* n, strlist* attrs);
int xml_tag_pred(xmlnode* node, void* arg);

static xmlnode*
xml_find_parent_predicate(xmlnode* node, int (*pred)(), void* vptr[]) {
  while((node = node->parent)) {
    if(pred(node, vptr[0], vptr[1], vptr[2]))
      break;
  }
  return node;
}

xmlnode*
xml_pfind_parent(xmlnode* node, int (*pred)(), void* ptr[]) {
  xmlnode* ret;
  strlist names, attrs;
  strlist_init(&names, '\0');
  strlist_init(&attrs, '\0');
  if(ptr[0]) {
    strlist_froms(&names, ptr[0], '|');
    ptr[0] = &names;
  }
  if(ptr[1]) {
    strlist_froms(&attrs, ptr[1], '|');
    ptr[1] = &attrs;
  }
  ret = xml_find_parent_predicate(node, pred, ptr);
  if(names.sa.a) strlist_free(&names);
  if(attrs.sa.a) strlist_free(&attrs);
  return ret;
}

xmlnode*
xml_find_parent_pred_1(xmlnode* node, int (*pred)(/*xmlnode*,void*,*/), void* arg) {
  void* vptr[] = {arg, NULL, NULL};
  return xml_pfind_parent(node, pred, vptr);
}

xmlnode*
xml_find_parent_pred_2(xmlnode* node, int (*pred)(/*xmlnode*,void*,void**/), void* a0, void* a1) {
  void* vptr[] = {a0, a1, NULL};
  return xml_pfind_parent(node, pred, vptr);
}
xmlnode*
xml_find_parent_pred_3(xmlnode* node, int (*pred)(/*xmlnode*,void*,void**/), void* a0, void* a1, void* a2) {
  void* vptr[] = {a0, a1, a2};
  return xml_pfind_parent(node, pred, vptr);
}


xmlnode*
xml_find_parent(xmlnode* node, const char* tag) {
  return xml_find_parent_pred_1(node, &xml_tag_pred, tag);
}

xmlnode*
xml_find_parent_attr(xmlnode* node, const char* attrs) {
  return xml_find_parent_pred_2(node, &xml_has_attr, NULL, attrs);
}