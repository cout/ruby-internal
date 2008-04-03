#include <ruby.h>

#ifdef RUBY_VM

static VALUE instruction_initialize(int argc, VALUE * argv, VALUE self)
{
  VALUE operands = rb_ary_new4(argc, argv);
  rb_iv_set(self, "@operands", operands);
  return Qnil;
}

static VALUE instruction_operands(VALUE self)
{
  return rb_iv_get(self, "@operands");
}

#endif

void Init_instruction(void)
{
#ifdef RUBY_VM
  VALUE rb_cInstruction = rb_define_class_under(rb_cVM, "Instruction", rb_cObject);
  rb_define_method(rb_cInstruction, "initialize", instruction_initialize, -1);
  rb_define_method(rb_cInstruction, "operands", instruction_operands, 0);
  rb_undef_method(rb_cInstruction, "new");

  define_instruction_subclasses(rb_cInstruction);
#endif
}

