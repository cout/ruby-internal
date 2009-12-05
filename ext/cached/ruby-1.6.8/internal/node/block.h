#ifndef ruby_internal_block__h_
#define ruby_internal_block__h_

#include <ruby.h>

#ifndef RUBY_VM
#include "env.h"
#endif

struct BLOCKTAG {
    struct RBasic super;
    long dst;
    long flags;
};

struct BLOCK {
    NODE *var;
    NODE *body;
    VALUE self;
    struct FRAME frame;
    struct SCOPE *scope;
    struct BLOCKTAG *tag;
    VALUE klass;
    int iter;
    int vmode;
    int flags;
    struct RVarmap *dyna_vars;
    VALUE orig_thread;
    VALUE wrapper;
    struct BLOCK *prev;
};


#endif

