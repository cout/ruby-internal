#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

extern rb_vm_t * ruby_current_vm;

/*
 * call-seq:
 *   RubyVM.current => RubyVM
 *
 * Returns the current ruby VM.
 */
static VALUE vm_current(VALUE klass)
{
  return ruby_current_vm->self;
}

/*
 * call-seq:
 *   ruby_vm.top_self
 *
 * Returns the VM's "top self".
 */
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
  VALUE rb_cRubyVM;

  if(!rb_const_defined(rb_cObject, rb_intern("RubyVM")))
  {
    rb_define_const(
        rb_cObject,
        "RubyVM",
        rb_const_get(rb_cObject, rb_intern("VM")));
  }

  rb_cRubyVM = rb_define_class("RubyVM", rb_cObject);
  rb_define_singleton_method(rb_cRubyVM, "current", vm_current, 0);
  rb_define_method(rb_cRubyVM, "top_self", vm_top_self, 0);
#endif
}

