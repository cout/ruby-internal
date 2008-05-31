#include "control_frame.h"
#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

static VALUE rb_cVmControlFrame;

#endif

void Init_control_frame(void)
{
#ifdef RUBY_VM
  /* For rdoc: rb_cVM = rb_define_class("VM", rb_cObject); */
  rb_cVmControlFrame = rb_define_class_under(rb_cVM, "ControlFrame", rb_cObject);
#endif
}

