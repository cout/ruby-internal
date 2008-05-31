#include "control_frame.h"
#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

static VALUE rb_cVmControlFrame;

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
  return ID2SYM(cfp->control_frame->method_id);
}

static VALUE control_frame_method_class(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return ID2SYM(cfp->control_frame->method_id);
}

#endif

void Init_control_frame(void)
{
#ifdef RUBY_VM
  /* For rdoc: rb_cVM = rb_define_class("VM", rb_cObject); */
  rb_cVmControlFrame = rb_define_class_under(rb_cVM, "ControlFrame", rb_cObject);
  rb_define_method(rb_cVmControlFrame, "iseq", control_frame_iseq, 0);
  rb_define_method(rb_cVmControlFrame, "self", control_frame_self, 0);
  rb_define_method(rb_cVmControlFrame, "block_iseq", control_frame_block_iseq, 0);
  rb_define_method(rb_cVmControlFrame, "proc", control_frame_proc, 0);
  rb_define_method(rb_cVmControlFrame, "method_id", control_frame_method_id, 0);
  rb_define_method(rb_cVmControlFrame, "method_class", control_frame_method_class, 0);
#endif
}

