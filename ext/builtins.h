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

#endif

