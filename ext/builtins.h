#ifndef BUILTINS_H
#define BUILTINS_H

/* Macros for manipulating builtins */

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

#ifndef RSTRING_LEN
#define RSTRING_LEN(a) RARRAY(a)->len
#endif

#ifndef RSTRING_PTR
#define RSTRING_PTR(a) RARRAY(a)->ptr
#endif

#endif

