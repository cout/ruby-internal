#ifndef ruby_internal_method__h_
#define ruby_internal_method__h_

#include <ruby.h>

#include "internal/node/ruby_internal_node.h"

struct METHOD {
    VALUE klass, rklass;
    VALUE recv;
    ID id, oid;
    NODE *body;
};

#define METHOD_OCLASS(m) m->klass
#define METHOD_RCLASS(m) m->rklass

#endif

