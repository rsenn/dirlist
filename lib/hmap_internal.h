#include "hmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define HMAP_DEBUG(...) do {printf( __VA_ARGS__ );}while(0)

#define VALIDATE_DB(_hdb, _key, _klen) do{  \
    if( _hdb == NULL ){                     \
        return HMAP_DB_EMPTY;               \
    }                                       \
    if( _key == NULL || _klen <= 0){        \
        return HMAP_KEY_EMPTY;              \
    }                                       \
}while(0)


#define HDB_LIST_APPEND(_first,_item)                                 \
{                                                                     \
   if((_first) == NULL)                                              \
   {                                                                  \
      (_first) = (_item)->prev = (_item)->next = (_item);             \
   }                                                                  \
   else                                                               \
   {                                                                  \
      (_item)->prev = (_first)->prev;                                 \
      (_item)->next = (_first);                                       \
      (_first)->prev->next = (_item);                                 \
      (_first)->prev = (_item);                                       \
   }                                                                  \
}
#define HDB_LIST_REMOVE(_first,_item)                                 \
{                                                                     \
   if((_first) == (_item))                                           \
   {                                                                  \
      if((_first)->next == (_first))                                 \
         (_first) = NULL;                                             \
      else                                                            \
      {                                                               \
         (_first) = (_item)->next;                                    \
         (_item)->next->prev = (_item)->prev;                         \
         (_item)->prev->next = (_item)->next;                         \
      }                                                               \
   }                                                                  \
   else                                                               \
   {                                                                  \
      (_item)->next->prev = (_item)->prev;                            \
      (_item)->prev->next = (_item)->next;                            \
   }                                                                  \
   (_item)->prev = (_item)->next = NULL;                              \
}

#define HDB_HASH_APPEND(_first,_item)                                 \
{                                                                     \
   if((_first) == NULL)                                              \
   {                                                                  \
      (_first) = (_item)->hash_prev = (_item)->hash_next = (_item);   \
   }                                                                  \
   else                                                               \
   {                                                                  \
      (_item)->hash_prev = (_first)->hash_prev;                       \
      (_item)->hash_next = (_first);                                  \
      (_first)->hash_prev->hash_next = (_item);                       \
      (_first)->hash_prev = (_item);                                  \
   }                                                                  \
}

#define HDB_HASH_REMOVE(_first,_item)                                 \
{                                                                     \
   if((_first) == (_item))                                           \
   {                                                                  \
      if((_first)->hash_next == (_first))                            \
         (_first) = NULL;                                             \
      else                                                            \
      {                                                               \
         (_first) = (_item)->hash_next;                               \
         (_item)->hash_next->hash_prev = (_item)->hash_prev;          \
         (_item)->hash_prev->hash_next = (_item)->hash_next;          \
      }                                                               \
   }                                                                  \
   else                                                               \
   {                                                                  \
      (_item)->hash_next->hash_prev = (_item)->hash_prev;             \
      (_item)->hash_prev->hash_next = (_item)->hash_next;             \
   }                                                                  \
   (_item)->hash_prev = (_item)->hash_next = NULL;                    \
}

#define IS_NEXT_HASH(_first, _curr){        \
    if( _curr->hash_next == _first ){       \
        break;                              \
    }                                       \
    _curr = _curr->hash_next;               \
}

#define IS_NEXT_LIST(_first, _curr){        \
    if( _curr->next == _first ){            \
        break;                              \
    }                                       \
    _curr = _curr->next;                    \
}

int hmap_free_data(TUPLE *tuple);
