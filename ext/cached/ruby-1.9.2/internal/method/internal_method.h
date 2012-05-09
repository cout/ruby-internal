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
    VALUE recv;
    VALUE rclass;
    ID id;
    rb_method_entry_t me;
};

#define METHOD_OCLASS(m) m->klass
#define METHOD_RCLASS(m) m->rclass

#endif

