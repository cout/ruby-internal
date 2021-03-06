#include "instruction.h"
#include "insns_info.h"
#include <ruby.h>

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

#ifdef RUBY_VM

/*
 * call-seq:
 *   Instruction.new(*operands) => Instruction
 *
 * Create a new instruction with the given operands.
 */
static VALUE instruction_initialize(int argc, VALUE * argv, VALUE self)
{
  VALUE operands = rb_ary_new4(argc, argv);
  rb_iv_set(self, "@operands", operands);
  return Qnil;
}

/*
 * call-seq:
 *   instruction.operands => Array
 *
 * Returns the instructions operands.
 */
static VALUE instruction_operands(VALUE self)
{
  return rb_iv_get(self, "@operands");
}

VALUE allocate_instruction(int instruction, VALUE args)
{
  return rb_class_new_instance(
      RARRAY_LEN(args),
      RARRAY_PTR(args),
      instruction_class[instruction]);
}

#endif

void Init_instruction(void)
{
#ifdef RUBY_VM
  VALUE rb_cRubyVM;
  VALUE rb_cInstruction;

  if(!rb_const_defined(rb_cObject, rb_intern("RubyVM")))
  {
    rb_define_const(
        rb_cObject,
        "RubyVM",
        rb_const_get(rb_cObject, rb_intern("VM")));
  }

  rb_cRubyVM = rb_define_class("RubyVM", rb_cObject);

  rb_cInstruction = rb_define_class_under(rb_cRubyVM, "Instruction", rb_cObject);
  rb_define_method(rb_cInstruction, "initialize", instruction_initialize, -1);
  rb_define_method(rb_cInstruction, "operands", instruction_operands, 0);
  rb_undef_method(rb_cInstruction, "new");

  define_instruction_subclasses(rb_cInstruction);

  /* Silence compiler warnings about unused static functions */
  insn_name(0);
  insn_op_type(0, 0);
  insn_op_types(0);
#endif
}

