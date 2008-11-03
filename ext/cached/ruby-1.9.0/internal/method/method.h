#ifndef ruby_internal_method__h_
#define ruby_internal_method__h_

#include <ruby.h>

#include "internal/node/ruby_internal_node.h"

struct METHOD {
    VALUE oclass;		/* class that holds the method */
    VALUE rclass;		/* class of the receiver */
    VALUE recv;
    ID id, oid;
    NODE *body;
};

#define METHOD_OCLASS(m) m->oclass
#define METHOD_RCLASS(m) m->rclass

#endif

