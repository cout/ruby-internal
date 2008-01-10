#ifndef evalinfo__h_
#define evalinfo__h_

#include "ruby_version.h"

#ifndef RUBY_HAS_YARV
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
    VALUE block_obj;
    struct BLOCK *outer;
    struct BLOCK *prev;
};

static void
compile_error(at)
    const char *at;
{
    VALUE str;

    ruby_nerrs = 0;
    str = rb_str_buf_new2("compile error");
    if (at) {
	rb_str_buf_cat2(str, " in ");
	rb_str_buf_cat2(str, at);
    }
    rb_str_buf_cat(str, "\n", 1);
    if (!NIL_P(ruby_errinfo)) {
	rb_str_append(str, rb_obj_as_string(ruby_errinfo));
    }
    rb_exc_raise(rb_exc_new3(rb_eSyntaxError, str));
}

struct METHOD {
    VALUE klass, rklass;
    VALUE recv;
    ID id, oid;
    NODE *body;
};


#endif

