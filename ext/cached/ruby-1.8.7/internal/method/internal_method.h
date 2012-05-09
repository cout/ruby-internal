#ifndef ruby_internal_method__h_
#define ruby_internal_method__h_

#include <ruby.h>

#ifdef RUBY_VM
#include "method.h"
#endif

#include "internal/node/ruby_internal_node.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

struct METHOD {
    VALUE klass, rklass;
    VALUE recv;
    ID id, oid;
    int safe_level;
    NODE *body;
};

#define METHOD_OCLASS(m) m->klass
#define METHOD_RCLASS(m) m->rklass

#endif

