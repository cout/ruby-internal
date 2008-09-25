#include <ruby.h>

#ifdef RUBY_VM
#include "vm_core.h"
#include "internal/vm/control_frame/control_frame.h"
#endif

#ifdef RUBY_VM

static VALUE rb_cVmControlFrame;

/* TODO: also defined in control_frame.c */
void mark_ruby_internal_control_frame(
    struct RubyInternalControlFrame * cfp)
{
  rb_gc_mark(cfp->thread);
}

static VALUE thread_cfp(VALUE self)
{
  rb_thread_t * th;
  rb_control_frame_t * prev_cfp;

  GetThreadPtr(self, th);

  /* TODO: Not sure how many control frames back to go to get the one we
   * want */
  prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(th->cfp);
  if((void *)(th->stack + th->stack_size) == (void *)prev_cfp)
  {
    return Qnil;
  }
  else
  {
    struct RubyInternalControlFrame * cfp;
    VALUE cfp_v;

    cfp_v = Data_Make_Struct(
        rb_cVmControlFrame,
        struct RubyInternalControlFrame,
        mark_ruby_internal_control_frame,
        free, 
        cfp);

    cfp->control_frame = prev_cfp;
    cfp->thread = self;

    return cfp_v;
  }
}

#endif

void Init_thread(void)
{
  /* For rdoc: rb_cThread = rb_define_class("Thread", rb_cObject); */

#ifdef RUBY_VM
  VALUE rb_cRubyVM;

  rb_require("internal/vm/control_frame");

  if(!rb_const_defined(rb_cObject, rb_intern("RubyVM")))
  {
    rb_define_const(
        rb_cObject,
        "RubyVM",
        rb_const_get(rb_cObject, rb_intern("VM")));
  }

  rb_cRubyVM = rb_define_class("RubyVM", rb_cObject);
  rb_cVmControlFrame = rb_const_get(rb_cRubyVM, rb_intern("ControlFrame"));

  rb_define_method(rb_cThread, "cfp", thread_cfp, 0);
#endif
}

