#include "control_frame.h"
#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

/* TODO: Using this class is dangerous, because the control frame can be
 * popped but a reference kept around to it.  It will still point to
 * valid memory, but the data itself will be invalid.
 */

static VALUE rb_cVmControlFrame;

void mark_ruby_internal_control_frame(
    struct RubyInternalControlFrame * cfp)
{
  rb_gc_mark(cfp->thread);
}

static VALUE control_frame_iseq(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return (VALUE)cfp->control_frame->iseq;
}

static VALUE control_frame_self(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return cfp->control_frame->self;
}

static VALUE control_frame_block_iseq(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  if(cfp->control_frame->proc)
  {
    return (VALUE)cfp->control_frame->block_iseq;
  }
  else
  {
    return Qfalse;
  }
}

static VALUE control_frame_proc(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return cfp->control_frame->proc;
}

static VALUE control_frame_method_id(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  if(cfp->control_frame->method_id)
  {
    return ID2SYM(cfp->control_frame->method_id);
  }
  else
  {
    return Qnil;
  }
}

static VALUE control_frame_method_class(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return cfp->control_frame->method_class;
}

static VALUE control_frame_prev(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  rb_thread_t * th;
  rb_control_frame_t * prev_cfp;

  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);

  GetThreadPtr(cfp->thread, th);

  prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp->control_frame);

  printf("current: %p\n", cfp->control_frame);
  printf("prev: %p\n", prev_cfp);
  printf("th->stack: %p\n", th->stack);
  printf("th->stack + th->stack_size: %p\n", th->stack + th->stack_size);

  if((void *)(th->stack + th->stack_size) == (void *)prev_cfp)
  {
    return Qnil;
  }
  else
  {
    VALUE cfp_v;
    struct RubyInternalControlFrame * new_cfp;

    cfp_v = Data_Make_Struct(
        rb_cVmControlFrame,
        struct RubyInternalControlFrame,
        mark_ruby_internal_control_frame,
        free,
        new_cfp);

    new_cfp->control_frame = prev_cfp;
    new_cfp->thread = cfp->thread;

    return cfp_v;
  }
}

#endif

void Init_control_frame(void)
{
#ifdef RUBY_VM
  VALUE rb_cRubyVM;

  if(!rb_const_defined(rb_cObject, rb_intern("RubyVM")))
  {
    rb_define_const(
        rb_cObject,
        "RubyVM",
        rb_const_get(rb_cObject, rb_intern("VM")));
  }

  rb_cRubyVM = rb_define_class("RubyVM", rb_cObject);
  rb_cVmControlFrame = rb_define_class_under(rb_cRubyVM, "ControlFrame", rb_cObject);
  rb_define_method(rb_cVmControlFrame, "iseq", control_frame_iseq, 0);
  rb_define_method(rb_cVmControlFrame, "self", control_frame_self, 0);
  rb_define_method(rb_cVmControlFrame, "block_iseq", control_frame_block_iseq, 0);
  rb_define_method(rb_cVmControlFrame, "proc", control_frame_proc, 0);
  rb_define_method(rb_cVmControlFrame, "method_id", control_frame_method_id, 0);
  rb_define_method(rb_cVmControlFrame, "method_class", control_frame_method_class, 0);
  rb_define_method(rb_cVmControlFrame, "prev", control_frame_prev, 0);
#endif
}

