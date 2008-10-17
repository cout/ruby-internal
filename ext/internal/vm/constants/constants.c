#include <ruby.h>

#ifdef RUBY_VM
#include "vm_core.h"
#endif

void Init_constants(void)
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
  rb_define_const(rb_cRubyVM, "CALL_ARGS_SPLAT_BIT", INT2NUM(VM_CALL_ARGS_SPLAT_BIT));
  rb_define_const(rb_cRubyVM, "CALL_ARGS_BLOCKARG_BIT", INT2NUM(VM_CALL_ARGS_BLOCKARG_BIT));
  rb_define_const(rb_cRubyVM, "CALL_FCALL_BIT", INT2NUM(VM_CALL_FCALL_BIT));
  rb_define_const(rb_cRubyVM, "CALL_VCALL_BIT", INT2NUM(VM_CALL_VCALL_BIT));
  rb_define_const(rb_cRubyVM, "CALL_TAILCALL_BIT", INT2NUM(VM_CALL_TAILCALL_BIT));
  rb_define_const(rb_cRubyVM, "CALL_TAILRECURSION_BIT", INT2NUM(VM_CALL_TAILRECURSION_BIT));
  rb_define_const(rb_cRubyVM, "CALL_SUPER_BIT", INT2NUM(VM_CALL_SUPER_BIT));
  rb_define_const(rb_cRubyVM, "CALL_SEND_BIT", INT2NUM(VM_CALL_SEND_BIT));
  rb_define_const(rb_cRubyVM, "SPECIAL_OBJECT_VMCORE", INT2NUM(SPECIAL_OBJECT_VMCORE));
  rb_define_const(rb_cRubyVM, "SPECIAL_OBJECT_CBASE", INT2NUM(SPECIAL_OBJECT_CBASE));
#endif
}

