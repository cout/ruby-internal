#include <ruby.h>

void Init_constants(void)
{
#ifdef RUBY_VM
  rb_define_const(rb_cVM, "CALL_ARGS_SPLAT_BIT", INT2NUM(VM_CALL_ARGS_SPLAT_BIT));
  rb_define_const(rb_cVM, "CALL_ARGS_BLOCKARG_BIT", INT2NUM(VM_CALL_ARGS_BLOCKARG_BIT));
  rb_define_const(rb_cVM, "CALL_FCALL_BIT", INT2NUM(VM_CALL_FCALL_BIT));
  rb_define_const(rb_cVM, "CALL_VCALL_BIT", INT2NUM(VM_CALL_VCALL_BIT));
  rb_define_const(rb_cVM, "CALL_TAILCALL_BIT", INT2NUM(VM_CALL_TAILCALL_BIT));
  rb_define_const(rb_cVM, "CALL_TAILRECURSION_BIT", INT2NUM(VM_CALL_TAILRECURSION_BIT));
  rb_define_const(rb_cVM, "CALL_SUPER_BIT", INT2NUM(VM_CALL_SUPER_BIT));
  rb_define_const(rb_cVM, "CALL_SEND_BIT", INT2NUM(VM_CALL_SEND_BIT));
#endif
}

