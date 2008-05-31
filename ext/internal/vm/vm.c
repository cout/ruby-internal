#include "vm_core.h"
#include <ruby.h>

#ifdef RUBY_VM

VALUE rb_cVM;

extern rb_vm_t * ruby_current_vm;

static VALUE vm_current(VALUE klass)
{
  return ruby_current_vm->self;
}

static VALUE vm_last_status(VALUE self)
{
  rb_vm_t * vm;
  
  Data_Get_Struct(self, rb_vm_t, vm);

  return vm->last_status;
}

static VALUE vm_running(VALUE self)
{
  rb_vm_t * vm;
  
  Data_Get_Struct(self, rb_vm_t, vm);

  return vm->running ? Qtrue : Qfalse;
}

static VALUE vm_top_self(VALUE self)
{
  rb_vm_t * vm;
  
  Data_Get_Struct(self, rb_vm_t, vm);

  return vm->top_self;
}

#endif

void Init_vm(void)
{
#ifdef RUBY_VM
  rb_cVM = rb_define_class("VM", rb_cObject);
  rb_define_singleton_method(rb_cVM, "last_status", vm_current, 0);
  rb_define_method(rb_cVM, "last_status", vm_last_status, 0);
  rb_define_method(rb_cVM, "running", vm_running, 0);
  rb_define_method(rb_cVM, "top_self", vm_top_self, 0);
#endif
}

