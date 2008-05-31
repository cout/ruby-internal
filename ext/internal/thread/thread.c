#include <ruby.h>

#ifdef RUBY_VM
#include "vm_core.h"
#include "internal/vm/control_frame/control_frame.h"
#endif

#ifdef RUBY_VM

static VALUE rb_cVmControlFrame;

void mark_ruby_internal_control_frame(
    struct RubyInternalControlFrame * cfp)
{
  rb_gc_mark(cfp->thread);
}

static VALUE thread_cfp(VALUE self)
{
  rb_thread_t * th;
  VALUE cfp_v;
  struct RubyInternalControlFrame * cfp;

  GetThreadPtr(self, th);

  cfp_v = Data_Make_Struct(
      rb_cVmControlFrame,
      struct RubyInternalControlFrame,
      mark_ruby_internal_control_frame,
      free, 
      cfp);

  cfp->control_frame = th->cfp;
  cfp->thread = self;

  return cfp_v;
}

#endif

void Init_thread(void)
{
  /* For rdoc: rb_cThread = rb_define_class("Thread", rb_cObject); */

#ifdef RUBY_VM
  rb_require("internal/vm/control_frame");

  /* For rdoc: rb_cVM = rb_define_class("VM", rb_cObject); */
  rb_cVmControlFrame = rb_const_get(rb_cVM, rb_intern("ControlFrame"));

  rb_define_method(rb_cThread, "cfp", thread_cfp, 0);
#endif
}

