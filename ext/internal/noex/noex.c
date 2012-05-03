#include <ruby.h>

#include "internal/node/ruby_internal_node.h"

#ifdef RUBY_VM
#include "method.h"
#endif

void Init_noex(void)
{
  /*
   * Document-module: Noex
   *
   * The Noex module contains all the Noex constants from node.h.  These
   * constants can be passed to Module#add_method as modifiers to the
   * method being added.
   * 
   * [+PUBLIC+]     Method is public.
   * [+UNDEF+]      Method is undefined.
   * [+PRIVATE+]    Method is private.
   * [+PROTECTED+]  Method is protected.
   *
   * The following are available in Ruby 1.8 and below:
   * [+CFUNC+]      Method is a C function.
   *
   * And these are available in Ruby 1.9 and above:
   * [+NOSUPER+]    ???
   * [+MASK+]       ???
   */
  VALUE rb_mNoex = rb_define_module("Noex");
  rb_define_const(rb_mNoex, "PUBLIC",    INT2NUM(NOEX_PUBLIC));
  rb_define_const(rb_mNoex, "UNDEF",     INT2NUM(NOEX_UNDEF));
  rb_define_const(rb_mNoex, "PRIVATE",   INT2NUM(NOEX_PRIVATE));
  rb_define_const(rb_mNoex, "PROTECTED", INT2NUM(NOEX_PROTECTED));

#ifdef NOEX_CFUNC
  rb_define_const(rb_mNoex, "CFUNC",     INT2NUM(NOEX_CFUNC));
#endif

#ifdef NOEX_NOSUPER
  rb_define_const(rb_mNoex, "NOSUPER",   INT2NUM(NOEX_NOSUPER));
#endif

#ifdef NOEX_MASK
  rb_define_const(rb_mNoex, "MASK",      INT2NUM(NOEX_MASK));
#endif
}

