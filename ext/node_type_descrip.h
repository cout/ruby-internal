#ifndef node_type_descrip__h
#define node_type_descrip__h

#include "nodeinfo.h"

typedef struct {
  enum node_type nt;
  enum Node_Elem_Name n1;
  enum Node_Elem_Name n2;
  enum Node_Elem_Name n3;
  char const * name;
} Node_Type_Descrip;

Node_Type_Descrip const * node_type_descrip(enum node_type nt);

#endif

