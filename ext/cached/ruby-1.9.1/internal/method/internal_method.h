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
    VALUE oclass;		/* class that holds the method */
    VALUE rclass;		/* class of the receiver */
    VALUE recv;
    ID id, oid;
    NODE *body;
};

#define METHOD_OCLASS(m) m->oclass
#define METHOD_RCLASS(m) m->rclass

#endif

