#ifndef BUILTINS_H
#define BUILTINS_H

#include "ruby.h"

/* Macros for manipulating builtins */

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

#ifndef RSTRING_LEN
#define RSTRING_LEN(a) RSTRING(a)->len
#endif

#ifndef RSTRING_PTR
#define RSTRING_PTR(a) RSTRING(a)->ptr
#endif

#ifndef RCLASS_SUPER
#define RCLASS_SUPER(c) RCLASS(c)->super
#endif

#ifndef RCLASS_IV_TBL
#define RCLASS_IV_TBL(c) RCLASS(c)->iv_tbl
#endif

#ifndef RCLASS_M_TBL
#define RCLASS_M_TBL(c) RCLASS(c)->m_tbl
#endif

#if RUBY_VERSION_CODE >= 190
#define METHOD_OCLASS(m) m->oclass
#elif RUBY_VERSION_CODE < 170
#define METHOD_OCLASS(m) m->oklass
#else
#define METHOD_OCLASS(m) m->klass
#endif

#if RUBY_VERSION_CODE >= 190
#define METHOD_RCLASS(m) m->rclass
#elif RUBY_VERSION_CODE < 170
#define METHOD_RCLASS(m) m->klass
#else
#define METHOD_RCLASS(m) m->rklass
#endif

#ifndef NEW_NODE
#define NEW_NODE(t,a0,a1,a2) \
rb_node_newnode((t),(VALUE)(a0),(VALUE)(a1),(VALUE)(a2))
#endif

#endif
