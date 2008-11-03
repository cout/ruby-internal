#include "insns_info.h"

#ifdef RUBY_VM

static rb_compile_option_t COMPILE_OPTION_DEFAULT = {
    OPT_INLINE_CONST_CACHE, /* int inline_const_cache; */
    OPT_PEEPHOLE_OPTIMIZATION, /* int peephole_optimization; */
    OPT_TAILCALL_OPTIMIZATION, /* int tailcall_optimization */
    OPT_SPECIALISED_INSTRUCTION, /* int specialized_instruction; */
    OPT_OPERANDS_UNIFICATION, /* int operands_unification; */
    OPT_INSTRUCTIONS_UNIFICATION, /* int instructions_unification; */
    OPT_STACK_CACHING, /* int stack_caching; */
    OPT_TRACE_INSTRUCTION, /* int trace_instruction */
};
static const rb_compile_option_t COMPILE_OPTION_FALSE = {0};
void
make_compile_option(rb_compile_option_t *option, VALUE opt)
{
    if (opt == Qnil) {
	*option = COMPILE_OPTION_DEFAULT;
    }
    else if (opt == Qfalse) {
	*option = COMPILE_OPTION_FALSE;
    }
    else if (opt == Qtrue) {
	memset(option, 1, sizeof(rb_compile_option_t));
    }
    else if (CLASS_OF(opt) == rb_cHash) {
	*option = COMPILE_OPTION_DEFAULT;

#define SET_COMPILE_OPTION(o, h, mem) \
  { VALUE flag = rb_hash_aref(h, ID2SYM(rb_intern(#mem))); \
      if (flag == Qtrue)  { o->mem = 1; } \
      else if (flag == Qfalse)  { o->mem = 0; } \
  }
#define SET_COMPILE_OPTION_NUM(o, h, mem) \
  { VALUE num = rb_hash_aref(opt, ID2SYM(rb_intern(#mem))); \
      if (!NIL_P(num)) o->mem = NUM2INT(num); \
  }
	SET_COMPILE_OPTION(option, opt, inline_const_cache);
	SET_COMPILE_OPTION(option, opt, peephole_optimization);
	SET_COMPILE_OPTION(option, opt, tailcall_optimization);
	SET_COMPILE_OPTION(option, opt, specialized_instruction);
	SET_COMPILE_OPTION(option, opt, operands_unification);
	SET_COMPILE_OPTION(option, opt, instructions_unification);
	SET_COMPILE_OPTION(option, opt, stack_caching);
	SET_COMPILE_OPTION(option, opt, trace_instruction);
	SET_COMPILE_OPTION_NUM(option, opt, debug_level);
#undef SET_COMPILE_OPTION
#undef SET_COMPILE_OPTION_NUM
    }
    else {
	rb_raise(rb_eTypeError, "Compile option must be Hash/true/false/nil");
    }
}

VALUE instruction_class[VM_INSTRUCTION_SIZE];

static ID operand_type_name_of(int operand_type)
{
  char const * retval = "????";

  switch(operand_type)
  {
    case TS_ISEQ: retval = "iseq"; break;
    case TS_GENTRY: retval = "gentry"; break;
    case TS_OFFSET: retval = "operand"; break;
    case TS_DINDEX: retval = "dindex"; break;
    case TS_VARIABLE: retval = "variable"; break;
    case TS_CDHASH: retval = "cdhash"; break;
    case TS_IC: retval = "ic"; break;
    case TS_ID: retval = "id"; break;
    case TS_VALUE: retval = "value"; break;
    case TS_LINDEX: retval = "lindex"; break;
    case TS_NUM: retval = "num"; break;
  }

  return rb_intern(retval);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_NOP_length(VALUE self)
{
  return LONG2NUM(insn_len(0));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_NOP_to_i(VALUE self)
{
  return NUM2LONG(0);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_NOP_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(0); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_NOP_name(VALUE self)
{
  return rb_intern("NOP");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_NOP_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETLOCAL_length(VALUE self)
{
  return LONG2NUM(insn_len(1));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETLOCAL_to_i(VALUE self)
{
  return NUM2LONG(1);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETLOCAL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(1); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETLOCAL_name(VALUE self)
{
  return rb_intern("GETLOCAL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETLOCAL_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("idx"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETLOCAL_length(VALUE self)
{
  return LONG2NUM(insn_len(2));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETLOCAL_to_i(VALUE self)
{
  return NUM2LONG(2);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETLOCAL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(2); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETLOCAL_name(VALUE self)
{
  return rb_intern("SETLOCAL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETLOCAL_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("idx"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETSPECIAL_length(VALUE self)
{
  return LONG2NUM(insn_len(3));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETSPECIAL_to_i(VALUE self)
{
  return NUM2LONG(3);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETSPECIAL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(3); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETSPECIAL_name(VALUE self)
{
  return rb_intern("GETSPECIAL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETSPECIAL_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("key")), ID2SYM(rb_intern("type"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETSPECIAL_length(VALUE self)
{
  return LONG2NUM(insn_len(4));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETSPECIAL_to_i(VALUE self)
{
  return NUM2LONG(4);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETSPECIAL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(4); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETSPECIAL_name(VALUE self)
{
  return rb_intern("SETSPECIAL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETSPECIAL_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("key"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETDYNAMIC_length(VALUE self)
{
  return LONG2NUM(insn_len(5));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETDYNAMIC_to_i(VALUE self)
{
  return NUM2LONG(5);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETDYNAMIC_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(5); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETDYNAMIC_name(VALUE self)
{
  return rb_intern("GETDYNAMIC");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETDYNAMIC_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("idx")), ID2SYM(rb_intern("level"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETDYNAMIC_length(VALUE self)
{
  return LONG2NUM(insn_len(6));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETDYNAMIC_to_i(VALUE self)
{
  return NUM2LONG(6);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETDYNAMIC_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(6); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETDYNAMIC_name(VALUE self)
{
  return rb_intern("SETDYNAMIC");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETDYNAMIC_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("idx")), ID2SYM(rb_intern("level"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETINSTANCEVARIABLE_length(VALUE self)
{
  return LONG2NUM(insn_len(7));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETINSTANCEVARIABLE_to_i(VALUE self)
{
  return NUM2LONG(7);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETINSTANCEVARIABLE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(7); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETINSTANCEVARIABLE_name(VALUE self)
{
  return rb_intern("GETINSTANCEVARIABLE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETINSTANCEVARIABLE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETINSTANCEVARIABLE_length(VALUE self)
{
  return LONG2NUM(insn_len(8));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETINSTANCEVARIABLE_to_i(VALUE self)
{
  return NUM2LONG(8);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETINSTANCEVARIABLE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(8); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETINSTANCEVARIABLE_name(VALUE self)
{
  return rb_intern("SETINSTANCEVARIABLE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETINSTANCEVARIABLE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETCLASSVARIABLE_length(VALUE self)
{
  return LONG2NUM(insn_len(9));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETCLASSVARIABLE_to_i(VALUE self)
{
  return NUM2LONG(9);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETCLASSVARIABLE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(9); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETCLASSVARIABLE_name(VALUE self)
{
  return rb_intern("GETCLASSVARIABLE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETCLASSVARIABLE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETCLASSVARIABLE_length(VALUE self)
{
  return LONG2NUM(insn_len(10));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETCLASSVARIABLE_to_i(VALUE self)
{
  return NUM2LONG(10);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETCLASSVARIABLE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(10); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETCLASSVARIABLE_name(VALUE self)
{
  return rb_intern("SETCLASSVARIABLE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETCLASSVARIABLE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETCONSTANT_length(VALUE self)
{
  return LONG2NUM(insn_len(11));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETCONSTANT_to_i(VALUE self)
{
  return NUM2LONG(11);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETCONSTANT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(11); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETCONSTANT_name(VALUE self)
{
  return rb_intern("GETCONSTANT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETCONSTANT_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETCONSTANT_length(VALUE self)
{
  return LONG2NUM(insn_len(12));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETCONSTANT_to_i(VALUE self)
{
  return NUM2LONG(12);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETCONSTANT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(12); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETCONSTANT_name(VALUE self)
{
  return rb_intern("SETCONSTANT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETCONSTANT_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETGLOBAL_length(VALUE self)
{
  return LONG2NUM(insn_len(13));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETGLOBAL_to_i(VALUE self)
{
  return NUM2LONG(13);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETGLOBAL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(13); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETGLOBAL_name(VALUE self)
{
  return rb_intern("GETGLOBAL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETGLOBAL_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("entry"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETGLOBAL_length(VALUE self)
{
  return LONG2NUM(insn_len(14));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETGLOBAL_to_i(VALUE self)
{
  return NUM2LONG(14);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETGLOBAL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(14); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETGLOBAL_name(VALUE self)
{
  return rb_intern("SETGLOBAL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETGLOBAL_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("entry"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_PUTNIL_length(VALUE self)
{
  return LONG2NUM(insn_len(15));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_PUTNIL_to_i(VALUE self)
{
  return NUM2LONG(15);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_PUTNIL_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(15); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_PUTNIL_name(VALUE self)
{
  return rb_intern("PUTNIL");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_PUTNIL_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_PUTSELF_length(VALUE self)
{
  return LONG2NUM(insn_len(16));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_PUTSELF_to_i(VALUE self)
{
  return NUM2LONG(16);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_PUTSELF_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(16); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_PUTSELF_name(VALUE self)
{
  return rb_intern("PUTSELF");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_PUTSELF_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_PUTOBJECT_length(VALUE self)
{
  return LONG2NUM(insn_len(17));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_PUTOBJECT_to_i(VALUE self)
{
  return NUM2LONG(17);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_PUTOBJECT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(17); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_PUTOBJECT_name(VALUE self)
{
  return rb_intern("PUTOBJECT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_PUTOBJECT_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("val"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_PUTSPECIALOBJECT_length(VALUE self)
{
  return LONG2NUM(insn_len(18));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_PUTSPECIALOBJECT_to_i(VALUE self)
{
  return NUM2LONG(18);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_PUTSPECIALOBJECT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(18); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_PUTSPECIALOBJECT_name(VALUE self)
{
  return rb_intern("PUTSPECIALOBJECT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_PUTSPECIALOBJECT_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("value_type"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_PUTISEQ_length(VALUE self)
{
  return LONG2NUM(insn_len(19));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_PUTISEQ_to_i(VALUE self)
{
  return NUM2LONG(19);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_PUTISEQ_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(19); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_PUTISEQ_name(VALUE self)
{
  return rb_intern("PUTISEQ");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_PUTISEQ_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("iseq"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_PUTSTRING_length(VALUE self)
{
  return LONG2NUM(insn_len(20));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_PUTSTRING_to_i(VALUE self)
{
  return NUM2LONG(20);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_PUTSTRING_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(20); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_PUTSTRING_name(VALUE self)
{
  return rb_intern("PUTSTRING");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_PUTSTRING_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("str"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_CONCATSTRINGS_length(VALUE self)
{
  return LONG2NUM(insn_len(21));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_CONCATSTRINGS_to_i(VALUE self)
{
  return NUM2LONG(21);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_CONCATSTRINGS_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(21); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_CONCATSTRINGS_name(VALUE self)
{
  return rb_intern("CONCATSTRINGS");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_CONCATSTRINGS_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("num"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_TOSTRING_length(VALUE self)
{
  return LONG2NUM(insn_len(22));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_TOSTRING_to_i(VALUE self)
{
  return NUM2LONG(22);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_TOSTRING_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(22); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_TOSTRING_name(VALUE self)
{
  return rb_intern("TOSTRING");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_TOSTRING_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_TOREGEXP_length(VALUE self)
{
  return LONG2NUM(insn_len(23));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_TOREGEXP_to_i(VALUE self)
{
  return NUM2LONG(23);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_TOREGEXP_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(23); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_TOREGEXP_name(VALUE self)
{
  return rb_intern("TOREGEXP");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_TOREGEXP_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("opt")), ID2SYM(rb_intern("cnt"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_NEWARRAY_length(VALUE self)
{
  return LONG2NUM(insn_len(24));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_NEWARRAY_to_i(VALUE self)
{
  return NUM2LONG(24);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_NEWARRAY_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(24); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_NEWARRAY_name(VALUE self)
{
  return rb_intern("NEWARRAY");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_NEWARRAY_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("num"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_DUPARRAY_length(VALUE self)
{
  return LONG2NUM(insn_len(25));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_DUPARRAY_to_i(VALUE self)
{
  return NUM2LONG(25);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_DUPARRAY_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(25); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_DUPARRAY_name(VALUE self)
{
  return rb_intern("DUPARRAY");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_DUPARRAY_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("ary"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_EXPANDARRAY_length(VALUE self)
{
  return LONG2NUM(insn_len(26));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_EXPANDARRAY_to_i(VALUE self)
{
  return NUM2LONG(26);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_EXPANDARRAY_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(26); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_EXPANDARRAY_name(VALUE self)
{
  return rb_intern("EXPANDARRAY");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_EXPANDARRAY_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("num")), ID2SYM(rb_intern("flag"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_CONCATARRAY_length(VALUE self)
{
  return LONG2NUM(insn_len(27));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_CONCATARRAY_to_i(VALUE self)
{
  return NUM2LONG(27);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_CONCATARRAY_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(27); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_CONCATARRAY_name(VALUE self)
{
  return rb_intern("CONCATARRAY");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_CONCATARRAY_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SPLATARRAY_length(VALUE self)
{
  return LONG2NUM(insn_len(28));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SPLATARRAY_to_i(VALUE self)
{
  return NUM2LONG(28);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SPLATARRAY_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(28); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SPLATARRAY_name(VALUE self)
{
  return rb_intern("SPLATARRAY");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SPLATARRAY_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("flag"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_CHECKINCLUDEARRAY_length(VALUE self)
{
  return LONG2NUM(insn_len(29));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_CHECKINCLUDEARRAY_to_i(VALUE self)
{
  return NUM2LONG(29);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_CHECKINCLUDEARRAY_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(29); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_CHECKINCLUDEARRAY_name(VALUE self)
{
  return rb_intern("CHECKINCLUDEARRAY");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_CHECKINCLUDEARRAY_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("flag"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_NEWHASH_length(VALUE self)
{
  return LONG2NUM(insn_len(30));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_NEWHASH_to_i(VALUE self)
{
  return NUM2LONG(30);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_NEWHASH_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(30); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_NEWHASH_name(VALUE self)
{
  return rb_intern("NEWHASH");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_NEWHASH_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("num"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_NEWRANGE_length(VALUE self)
{
  return LONG2NUM(insn_len(31));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_NEWRANGE_to_i(VALUE self)
{
  return NUM2LONG(31);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_NEWRANGE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(31); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_NEWRANGE_name(VALUE self)
{
  return rb_intern("NEWRANGE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_NEWRANGE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("flag"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_POP_length(VALUE self)
{
  return LONG2NUM(insn_len(32));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_POP_to_i(VALUE self)
{
  return NUM2LONG(32);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_POP_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(32); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_POP_name(VALUE self)
{
  return rb_intern("POP");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_POP_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_DUP_length(VALUE self)
{
  return LONG2NUM(insn_len(33));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_DUP_to_i(VALUE self)
{
  return NUM2LONG(33);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_DUP_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(33); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_DUP_name(VALUE self)
{
  return rb_intern("DUP");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_DUP_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_DUPN_length(VALUE self)
{
  return LONG2NUM(insn_len(34));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_DUPN_to_i(VALUE self)
{
  return NUM2LONG(34);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_DUPN_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(34); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_DUPN_name(VALUE self)
{
  return rb_intern("DUPN");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_DUPN_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("n"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SWAP_length(VALUE self)
{
  return LONG2NUM(insn_len(35));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SWAP_to_i(VALUE self)
{
  return NUM2LONG(35);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SWAP_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(35); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SWAP_name(VALUE self)
{
  return rb_intern("SWAP");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SWAP_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_REPUT_length(VALUE self)
{
  return LONG2NUM(insn_len(36));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_REPUT_to_i(VALUE self)
{
  return NUM2LONG(36);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_REPUT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(36); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_REPUT_name(VALUE self)
{
  return rb_intern("REPUT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_REPUT_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_TOPN_length(VALUE self)
{
  return LONG2NUM(insn_len(37));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_TOPN_to_i(VALUE self)
{
  return NUM2LONG(37);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_TOPN_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(37); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_TOPN_name(VALUE self)
{
  return rb_intern("TOPN");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_TOPN_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("n"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETN_length(VALUE self)
{
  return LONG2NUM(insn_len(38));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETN_to_i(VALUE self)
{
  return NUM2LONG(38);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETN_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(38); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETN_name(VALUE self)
{
  return rb_intern("SETN");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETN_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("n"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_ADJUSTSTACK_length(VALUE self)
{
  return LONG2NUM(insn_len(39));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_ADJUSTSTACK_to_i(VALUE self)
{
  return NUM2LONG(39);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_ADJUSTSTACK_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(39); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_ADJUSTSTACK_name(VALUE self)
{
  return rb_intern("ADJUSTSTACK");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_ADJUSTSTACK_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("n"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_DEFINED_length(VALUE self)
{
  return LONG2NUM(insn_len(40));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_DEFINED_to_i(VALUE self)
{
  return NUM2LONG(40);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_DEFINED_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(40); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_DEFINED_name(VALUE self)
{
  return rb_intern("DEFINED");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_DEFINED_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("type")), ID2SYM(rb_intern("obj")), ID2SYM(rb_intern("needstr"))
  };
  return rb_ary_new4(3, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_TRACE_length(VALUE self)
{
  return LONG2NUM(insn_len(41));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_TRACE_to_i(VALUE self)
{
  return NUM2LONG(41);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_TRACE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(41); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_TRACE_name(VALUE self)
{
  return rb_intern("TRACE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_TRACE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("nf"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_DEFINECLASS_length(VALUE self)
{
  return LONG2NUM(insn_len(42));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_DEFINECLASS_to_i(VALUE self)
{
  return NUM2LONG(42);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_DEFINECLASS_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(42); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_DEFINECLASS_name(VALUE self)
{
  return rb_intern("DEFINECLASS");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_DEFINECLASS_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("id")), ID2SYM(rb_intern("class_iseq")), ID2SYM(rb_intern("define_type"))
  };
  return rb_ary_new4(3, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SEND_length(VALUE self)
{
  return LONG2NUM(insn_len(43));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SEND_to_i(VALUE self)
{
  return NUM2LONG(43);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SEND_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(43); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SEND_name(VALUE self)
{
  return rb_intern("SEND");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SEND_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("op_id")), ID2SYM(rb_intern("op_argc")), ID2SYM(rb_intern("blockiseq")), ID2SYM(rb_intern("op_flag")), ID2SYM(rb_intern("ic"))
  };
  return rb_ary_new4(5, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_INVOKESUPER_length(VALUE self)
{
  return LONG2NUM(insn_len(44));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_INVOKESUPER_to_i(VALUE self)
{
  return NUM2LONG(44);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_INVOKESUPER_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(44); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_INVOKESUPER_name(VALUE self)
{
  return rb_intern("INVOKESUPER");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_INVOKESUPER_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("op_argc")), ID2SYM(rb_intern("blockiseq")), ID2SYM(rb_intern("op_flag"))
  };
  return rb_ary_new4(3, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_INVOKEBLOCK_length(VALUE self)
{
  return LONG2NUM(insn_len(45));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_INVOKEBLOCK_to_i(VALUE self)
{
  return NUM2LONG(45);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_INVOKEBLOCK_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(45); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_INVOKEBLOCK_name(VALUE self)
{
  return rb_intern("INVOKEBLOCK");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_INVOKEBLOCK_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("num")), ID2SYM(rb_intern("flag"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_LEAVE_length(VALUE self)
{
  return LONG2NUM(insn_len(46));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_LEAVE_to_i(VALUE self)
{
  return NUM2LONG(46);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_LEAVE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(46); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_LEAVE_name(VALUE self)
{
  return rb_intern("LEAVE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_LEAVE_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_FINISH_length(VALUE self)
{
  return LONG2NUM(insn_len(47));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_FINISH_to_i(VALUE self)
{
  return NUM2LONG(47);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_FINISH_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(47); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_FINISH_name(VALUE self)
{
  return rb_intern("FINISH");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_FINISH_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_THROW_length(VALUE self)
{
  return LONG2NUM(insn_len(48));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_THROW_to_i(VALUE self)
{
  return NUM2LONG(48);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_THROW_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(48); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_THROW_name(VALUE self)
{
  return rb_intern("THROW");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_THROW_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("throw_state"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_JUMP_length(VALUE self)
{
  return LONG2NUM(insn_len(49));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_JUMP_to_i(VALUE self)
{
  return NUM2LONG(49);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_JUMP_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(49); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_JUMP_name(VALUE self)
{
  return rb_intern("JUMP");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_JUMP_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("dst"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_BRANCHIF_length(VALUE self)
{
  return LONG2NUM(insn_len(50));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_BRANCHIF_to_i(VALUE self)
{
  return NUM2LONG(50);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_BRANCHIF_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(50); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_BRANCHIF_name(VALUE self)
{
  return rb_intern("BRANCHIF");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_BRANCHIF_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("dst"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_BRANCHUNLESS_length(VALUE self)
{
  return LONG2NUM(insn_len(51));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_BRANCHUNLESS_to_i(VALUE self)
{
  return NUM2LONG(51);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_BRANCHUNLESS_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(51); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_BRANCHUNLESS_name(VALUE self)
{
  return rb_intern("BRANCHUNLESS");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_BRANCHUNLESS_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("dst"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_GETINLINECACHE_length(VALUE self)
{
  return LONG2NUM(insn_len(52));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_GETINLINECACHE_to_i(VALUE self)
{
  return NUM2LONG(52);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_GETINLINECACHE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(52); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_GETINLINECACHE_name(VALUE self)
{
  return rb_intern("GETINLINECACHE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_GETINLINECACHE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("ic")), ID2SYM(rb_intern("dst"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_ONCEINLINECACHE_length(VALUE self)
{
  return LONG2NUM(insn_len(53));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_ONCEINLINECACHE_to_i(VALUE self)
{
  return NUM2LONG(53);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_ONCEINLINECACHE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(53); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_ONCEINLINECACHE_name(VALUE self)
{
  return rb_intern("ONCEINLINECACHE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_ONCEINLINECACHE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("ic")), ID2SYM(rb_intern("dst"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_SETINLINECACHE_length(VALUE self)
{
  return LONG2NUM(insn_len(54));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_SETINLINECACHE_to_i(VALUE self)
{
  return NUM2LONG(54);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_SETINLINECACHE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(54); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_SETINLINECACHE_name(VALUE self)
{
  return rb_intern("SETINLINECACHE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_SETINLINECACHE_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("dst"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_CASE_DISPATCH_length(VALUE self)
{
  return LONG2NUM(insn_len(55));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_CASE_DISPATCH_to_i(VALUE self)
{
  return NUM2LONG(55);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_CASE_DISPATCH_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(55); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_CASE_DISPATCH_name(VALUE self)
{
  return rb_intern("OPT_CASE_DISPATCH");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_CASE_DISPATCH_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("hash")), ID2SYM(rb_intern("else_offset"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_CHECKENV_length(VALUE self)
{
  return LONG2NUM(insn_len(56));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_CHECKENV_to_i(VALUE self)
{
  return NUM2LONG(56);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_CHECKENV_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(56); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_CHECKENV_name(VALUE self)
{
  return rb_intern("OPT_CHECKENV");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_CHECKENV_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_PLUS_length(VALUE self)
{
  return LONG2NUM(insn_len(57));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_PLUS_to_i(VALUE self)
{
  return NUM2LONG(57);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_PLUS_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(57); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_PLUS_name(VALUE self)
{
  return rb_intern("OPT_PLUS");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_PLUS_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_MINUS_length(VALUE self)
{
  return LONG2NUM(insn_len(58));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_MINUS_to_i(VALUE self)
{
  return NUM2LONG(58);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_MINUS_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(58); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_MINUS_name(VALUE self)
{
  return rb_intern("OPT_MINUS");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_MINUS_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_MULT_length(VALUE self)
{
  return LONG2NUM(insn_len(59));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_MULT_to_i(VALUE self)
{
  return NUM2LONG(59);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_MULT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(59); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_MULT_name(VALUE self)
{
  return rb_intern("OPT_MULT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_MULT_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_DIV_length(VALUE self)
{
  return LONG2NUM(insn_len(60));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_DIV_to_i(VALUE self)
{
  return NUM2LONG(60);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_DIV_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(60); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_DIV_name(VALUE self)
{
  return rb_intern("OPT_DIV");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_DIV_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_MOD_length(VALUE self)
{
  return LONG2NUM(insn_len(61));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_MOD_to_i(VALUE self)
{
  return NUM2LONG(61);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_MOD_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(61); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_MOD_name(VALUE self)
{
  return rb_intern("OPT_MOD");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_MOD_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_EQ_length(VALUE self)
{
  return LONG2NUM(insn_len(62));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_EQ_to_i(VALUE self)
{
  return NUM2LONG(62);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_EQ_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(62); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_EQ_name(VALUE self)
{
  return rb_intern("OPT_EQ");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_EQ_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("ic"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_NEQ_length(VALUE self)
{
  return LONG2NUM(insn_len(63));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_NEQ_to_i(VALUE self)
{
  return NUM2LONG(63);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_NEQ_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(63); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_NEQ_name(VALUE self)
{
  return rb_intern("OPT_NEQ");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_NEQ_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("ic1")), ID2SYM(rb_intern("ic2"))
  };
  return rb_ary_new4(2, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_LT_length(VALUE self)
{
  return LONG2NUM(insn_len(64));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_LT_to_i(VALUE self)
{
  return NUM2LONG(64);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_LT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(64); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_LT_name(VALUE self)
{
  return rb_intern("OPT_LT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_LT_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_LE_length(VALUE self)
{
  return LONG2NUM(insn_len(65));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_LE_to_i(VALUE self)
{
  return NUM2LONG(65);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_LE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(65); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_LE_name(VALUE self)
{
  return rb_intern("OPT_LE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_LE_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_GT_length(VALUE self)
{
  return LONG2NUM(insn_len(66));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_GT_to_i(VALUE self)
{
  return NUM2LONG(66);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_GT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(66); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_GT_name(VALUE self)
{
  return rb_intern("OPT_GT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_GT_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_GE_length(VALUE self)
{
  return LONG2NUM(insn_len(67));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_GE_to_i(VALUE self)
{
  return NUM2LONG(67);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_GE_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(67); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_GE_name(VALUE self)
{
  return rb_intern("OPT_GE");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_GE_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_LTLT_length(VALUE self)
{
  return LONG2NUM(insn_len(68));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_LTLT_to_i(VALUE self)
{
  return NUM2LONG(68);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_LTLT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(68); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_LTLT_name(VALUE self)
{
  return rb_intern("OPT_LTLT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_LTLT_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_AREF_length(VALUE self)
{
  return LONG2NUM(insn_len(69));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_AREF_to_i(VALUE self)
{
  return NUM2LONG(69);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_AREF_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(69); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_AREF_name(VALUE self)
{
  return rb_intern("OPT_AREF");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_AREF_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_ASET_length(VALUE self)
{
  return LONG2NUM(insn_len(70));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_ASET_to_i(VALUE self)
{
  return NUM2LONG(70);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_ASET_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(70); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_ASET_name(VALUE self)
{
  return rb_intern("OPT_ASET");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_ASET_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_LENGTH_length(VALUE self)
{
  return LONG2NUM(insn_len(71));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_LENGTH_to_i(VALUE self)
{
  return NUM2LONG(71);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_LENGTH_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(71); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_LENGTH_name(VALUE self)
{
  return rb_intern("OPT_LENGTH");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_LENGTH_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_SUCC_length(VALUE self)
{
  return LONG2NUM(insn_len(72));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_SUCC_to_i(VALUE self)
{
  return NUM2LONG(72);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_SUCC_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(72); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_SUCC_name(VALUE self)
{
  return rb_intern("OPT_SUCC");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_SUCC_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_NOT_length(VALUE self)
{
  return LONG2NUM(insn_len(73));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_NOT_to_i(VALUE self)
{
  return NUM2LONG(73);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_NOT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(73); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_NOT_name(VALUE self)
{
  return rb_intern("OPT_NOT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_NOT_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("ic"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_REGEXPMATCH1_length(VALUE self)
{
  return LONG2NUM(insn_len(74));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_REGEXPMATCH1_to_i(VALUE self)
{
  return NUM2LONG(74);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_REGEXPMATCH1_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(74); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_REGEXPMATCH1_name(VALUE self)
{
  return rb_intern("OPT_REGEXPMATCH1");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_REGEXPMATCH1_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("r"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_REGEXPMATCH2_length(VALUE self)
{
  return LONG2NUM(insn_len(75));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_REGEXPMATCH2_to_i(VALUE self)
{
  return NUM2LONG(75);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_REGEXPMATCH2_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(75); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_REGEXPMATCH2_name(VALUE self)
{
  return rb_intern("OPT_REGEXPMATCH2");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_REGEXPMATCH2_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_OPT_CALL_C_FUNCTION_length(VALUE self)
{
  return LONG2NUM(insn_len(76));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_OPT_CALL_C_FUNCTION_to_i(VALUE self)
{
  return NUM2LONG(76);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_OPT_CALL_C_FUNCTION_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(76); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_OPT_CALL_C_FUNCTION_name(VALUE self)
{
  return rb_intern("OPT_CALL_C_FUNCTION");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_OPT_CALL_C_FUNCTION_operand_names(VALUE self)
{
  VALUE v[] = {
    ID2SYM(rb_intern("funcptr"))
  };
  return rb_ary_new4(1, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_BITBLT_length(VALUE self)
{
  return LONG2NUM(insn_len(77));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_BITBLT_to_i(VALUE self)
{
  return NUM2LONG(77);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_BITBLT_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(77); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_BITBLT_name(VALUE self)
{
  return rb_intern("BITBLT");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_BITBLT_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}

/*
 * call-seq:
 *   insn.length => Integer
 *
 * Returns the length of the instruction.
 */
static VALUE insn_ANSWER_length(VALUE self)
{
  return LONG2NUM(insn_len(78));
}

/*
 * call-seq:
 *   insn.to_i => Integer
 *
 * Returns the numeric instruction number.
 */
static VALUE insn_ANSWER_to_i(VALUE self)
{
  return NUM2LONG(78);
}

/*
 * call-seq:
 *   insn.operand_types => Array of Symbol
 *
 * Returns an array containing the types of the operands.
 */
static VALUE insn_ANSWER_operand_types(VALUE self)
{
  VALUE operand_types = rb_ary_new();
  char const * s;

  for(s = insn_op_types(78); *s != '\0'; ++s)
  {
    rb_ary_push(operand_types, ID2SYM(operand_type_name_of(*s)));
  }

  return operand_types;
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the instruction.
 */
static VALUE insn_ANSWER_name(VALUE self)
{
  return rb_intern("ANSWER");
}

/*
 * call-seq:
 *   insn.name => Symbol
 *
 * Returns the name of the operands for this instruction.
 */
static VALUE insn_ANSWER_operand_names(VALUE self)
{
  VALUE v[] = {
    
  };
  return rb_ary_new4(0, v);
}


void define_instruction_subclasses(VALUE rb_cInstruction)
{
  /* For rdoc: rb_cVM = rb_define_class("VM", rb_cObject); */
  /* For rdoc: rb_cInstruction = rb_define_class_under(rb_cVM, "Instruction", rb_cObject); */

  {
    VALUE rb_cInstruction_NOP = instruction_class[0] = rb_define_class_under(rb_cInstruction, "NOP", rb_cInstruction);
    rb_define_method(rb_cInstruction_NOP, "length", insn_NOP_length, 0);
    rb_define_method(rb_cInstruction_NOP, "to_i", insn_NOP_to_i, 0);
    rb_define_method(rb_cInstruction_NOP, "operand_types", insn_NOP_operand_types, 0);
    rb_define_method(rb_cInstruction_NOP, "operand_names", insn_NOP_operand_names, 0);
    rb_define_method(rb_cInstruction_NOP, "name", insn_NOP_name, 0);
  }
  {
    VALUE rb_cInstruction_GETLOCAL = instruction_class[1] = rb_define_class_under(rb_cInstruction, "GETLOCAL", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETLOCAL, "length", insn_GETLOCAL_length, 0);
    rb_define_method(rb_cInstruction_GETLOCAL, "to_i", insn_GETLOCAL_to_i, 0);
    rb_define_method(rb_cInstruction_GETLOCAL, "operand_types", insn_GETLOCAL_operand_types, 0);
    rb_define_method(rb_cInstruction_GETLOCAL, "operand_names", insn_GETLOCAL_operand_names, 0);
    rb_define_method(rb_cInstruction_GETLOCAL, "name", insn_GETLOCAL_name, 0);
  }
  {
    VALUE rb_cInstruction_SETLOCAL = instruction_class[2] = rb_define_class_under(rb_cInstruction, "SETLOCAL", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETLOCAL, "length", insn_SETLOCAL_length, 0);
    rb_define_method(rb_cInstruction_SETLOCAL, "to_i", insn_SETLOCAL_to_i, 0);
    rb_define_method(rb_cInstruction_SETLOCAL, "operand_types", insn_SETLOCAL_operand_types, 0);
    rb_define_method(rb_cInstruction_SETLOCAL, "operand_names", insn_SETLOCAL_operand_names, 0);
    rb_define_method(rb_cInstruction_SETLOCAL, "name", insn_SETLOCAL_name, 0);
  }
  {
    VALUE rb_cInstruction_GETSPECIAL = instruction_class[3] = rb_define_class_under(rb_cInstruction, "GETSPECIAL", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETSPECIAL, "length", insn_GETSPECIAL_length, 0);
    rb_define_method(rb_cInstruction_GETSPECIAL, "to_i", insn_GETSPECIAL_to_i, 0);
    rb_define_method(rb_cInstruction_GETSPECIAL, "operand_types", insn_GETSPECIAL_operand_types, 0);
    rb_define_method(rb_cInstruction_GETSPECIAL, "operand_names", insn_GETSPECIAL_operand_names, 0);
    rb_define_method(rb_cInstruction_GETSPECIAL, "name", insn_GETSPECIAL_name, 0);
  }
  {
    VALUE rb_cInstruction_SETSPECIAL = instruction_class[4] = rb_define_class_under(rb_cInstruction, "SETSPECIAL", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETSPECIAL, "length", insn_SETSPECIAL_length, 0);
    rb_define_method(rb_cInstruction_SETSPECIAL, "to_i", insn_SETSPECIAL_to_i, 0);
    rb_define_method(rb_cInstruction_SETSPECIAL, "operand_types", insn_SETSPECIAL_operand_types, 0);
    rb_define_method(rb_cInstruction_SETSPECIAL, "operand_names", insn_SETSPECIAL_operand_names, 0);
    rb_define_method(rb_cInstruction_SETSPECIAL, "name", insn_SETSPECIAL_name, 0);
  }
  {
    VALUE rb_cInstruction_GETDYNAMIC = instruction_class[5] = rb_define_class_under(rb_cInstruction, "GETDYNAMIC", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETDYNAMIC, "length", insn_GETDYNAMIC_length, 0);
    rb_define_method(rb_cInstruction_GETDYNAMIC, "to_i", insn_GETDYNAMIC_to_i, 0);
    rb_define_method(rb_cInstruction_GETDYNAMIC, "operand_types", insn_GETDYNAMIC_operand_types, 0);
    rb_define_method(rb_cInstruction_GETDYNAMIC, "operand_names", insn_GETDYNAMIC_operand_names, 0);
    rb_define_method(rb_cInstruction_GETDYNAMIC, "name", insn_GETDYNAMIC_name, 0);
  }
  {
    VALUE rb_cInstruction_SETDYNAMIC = instruction_class[6] = rb_define_class_under(rb_cInstruction, "SETDYNAMIC", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETDYNAMIC, "length", insn_SETDYNAMIC_length, 0);
    rb_define_method(rb_cInstruction_SETDYNAMIC, "to_i", insn_SETDYNAMIC_to_i, 0);
    rb_define_method(rb_cInstruction_SETDYNAMIC, "operand_types", insn_SETDYNAMIC_operand_types, 0);
    rb_define_method(rb_cInstruction_SETDYNAMIC, "operand_names", insn_SETDYNAMIC_operand_names, 0);
    rb_define_method(rb_cInstruction_SETDYNAMIC, "name", insn_SETDYNAMIC_name, 0);
  }
  {
    VALUE rb_cInstruction_GETINSTANCEVARIABLE = instruction_class[7] = rb_define_class_under(rb_cInstruction, "GETINSTANCEVARIABLE", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETINSTANCEVARIABLE, "length", insn_GETINSTANCEVARIABLE_length, 0);
    rb_define_method(rb_cInstruction_GETINSTANCEVARIABLE, "to_i", insn_GETINSTANCEVARIABLE_to_i, 0);
    rb_define_method(rb_cInstruction_GETINSTANCEVARIABLE, "operand_types", insn_GETINSTANCEVARIABLE_operand_types, 0);
    rb_define_method(rb_cInstruction_GETINSTANCEVARIABLE, "operand_names", insn_GETINSTANCEVARIABLE_operand_names, 0);
    rb_define_method(rb_cInstruction_GETINSTANCEVARIABLE, "name", insn_GETINSTANCEVARIABLE_name, 0);
  }
  {
    VALUE rb_cInstruction_SETINSTANCEVARIABLE = instruction_class[8] = rb_define_class_under(rb_cInstruction, "SETINSTANCEVARIABLE", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETINSTANCEVARIABLE, "length", insn_SETINSTANCEVARIABLE_length, 0);
    rb_define_method(rb_cInstruction_SETINSTANCEVARIABLE, "to_i", insn_SETINSTANCEVARIABLE_to_i, 0);
    rb_define_method(rb_cInstruction_SETINSTANCEVARIABLE, "operand_types", insn_SETINSTANCEVARIABLE_operand_types, 0);
    rb_define_method(rb_cInstruction_SETINSTANCEVARIABLE, "operand_names", insn_SETINSTANCEVARIABLE_operand_names, 0);
    rb_define_method(rb_cInstruction_SETINSTANCEVARIABLE, "name", insn_SETINSTANCEVARIABLE_name, 0);
  }
  {
    VALUE rb_cInstruction_GETCLASSVARIABLE = instruction_class[9] = rb_define_class_under(rb_cInstruction, "GETCLASSVARIABLE", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETCLASSVARIABLE, "length", insn_GETCLASSVARIABLE_length, 0);
    rb_define_method(rb_cInstruction_GETCLASSVARIABLE, "to_i", insn_GETCLASSVARIABLE_to_i, 0);
    rb_define_method(rb_cInstruction_GETCLASSVARIABLE, "operand_types", insn_GETCLASSVARIABLE_operand_types, 0);
    rb_define_method(rb_cInstruction_GETCLASSVARIABLE, "operand_names", insn_GETCLASSVARIABLE_operand_names, 0);
    rb_define_method(rb_cInstruction_GETCLASSVARIABLE, "name", insn_GETCLASSVARIABLE_name, 0);
  }
  {
    VALUE rb_cInstruction_SETCLASSVARIABLE = instruction_class[10] = rb_define_class_under(rb_cInstruction, "SETCLASSVARIABLE", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETCLASSVARIABLE, "length", insn_SETCLASSVARIABLE_length, 0);
    rb_define_method(rb_cInstruction_SETCLASSVARIABLE, "to_i", insn_SETCLASSVARIABLE_to_i, 0);
    rb_define_method(rb_cInstruction_SETCLASSVARIABLE, "operand_types", insn_SETCLASSVARIABLE_operand_types, 0);
    rb_define_method(rb_cInstruction_SETCLASSVARIABLE, "operand_names", insn_SETCLASSVARIABLE_operand_names, 0);
    rb_define_method(rb_cInstruction_SETCLASSVARIABLE, "name", insn_SETCLASSVARIABLE_name, 0);
  }
  {
    VALUE rb_cInstruction_GETCONSTANT = instruction_class[11] = rb_define_class_under(rb_cInstruction, "GETCONSTANT", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETCONSTANT, "length", insn_GETCONSTANT_length, 0);
    rb_define_method(rb_cInstruction_GETCONSTANT, "to_i", insn_GETCONSTANT_to_i, 0);
    rb_define_method(rb_cInstruction_GETCONSTANT, "operand_types", insn_GETCONSTANT_operand_types, 0);
    rb_define_method(rb_cInstruction_GETCONSTANT, "operand_names", insn_GETCONSTANT_operand_names, 0);
    rb_define_method(rb_cInstruction_GETCONSTANT, "name", insn_GETCONSTANT_name, 0);
  }
  {
    VALUE rb_cInstruction_SETCONSTANT = instruction_class[12] = rb_define_class_under(rb_cInstruction, "SETCONSTANT", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETCONSTANT, "length", insn_SETCONSTANT_length, 0);
    rb_define_method(rb_cInstruction_SETCONSTANT, "to_i", insn_SETCONSTANT_to_i, 0);
    rb_define_method(rb_cInstruction_SETCONSTANT, "operand_types", insn_SETCONSTANT_operand_types, 0);
    rb_define_method(rb_cInstruction_SETCONSTANT, "operand_names", insn_SETCONSTANT_operand_names, 0);
    rb_define_method(rb_cInstruction_SETCONSTANT, "name", insn_SETCONSTANT_name, 0);
  }
  {
    VALUE rb_cInstruction_GETGLOBAL = instruction_class[13] = rb_define_class_under(rb_cInstruction, "GETGLOBAL", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETGLOBAL, "length", insn_GETGLOBAL_length, 0);
    rb_define_method(rb_cInstruction_GETGLOBAL, "to_i", insn_GETGLOBAL_to_i, 0);
    rb_define_method(rb_cInstruction_GETGLOBAL, "operand_types", insn_GETGLOBAL_operand_types, 0);
    rb_define_method(rb_cInstruction_GETGLOBAL, "operand_names", insn_GETGLOBAL_operand_names, 0);
    rb_define_method(rb_cInstruction_GETGLOBAL, "name", insn_GETGLOBAL_name, 0);
  }
  {
    VALUE rb_cInstruction_SETGLOBAL = instruction_class[14] = rb_define_class_under(rb_cInstruction, "SETGLOBAL", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETGLOBAL, "length", insn_SETGLOBAL_length, 0);
    rb_define_method(rb_cInstruction_SETGLOBAL, "to_i", insn_SETGLOBAL_to_i, 0);
    rb_define_method(rb_cInstruction_SETGLOBAL, "operand_types", insn_SETGLOBAL_operand_types, 0);
    rb_define_method(rb_cInstruction_SETGLOBAL, "operand_names", insn_SETGLOBAL_operand_names, 0);
    rb_define_method(rb_cInstruction_SETGLOBAL, "name", insn_SETGLOBAL_name, 0);
  }
  {
    VALUE rb_cInstruction_PUTNIL = instruction_class[15] = rb_define_class_under(rb_cInstruction, "PUTNIL", rb_cInstruction);
    rb_define_method(rb_cInstruction_PUTNIL, "length", insn_PUTNIL_length, 0);
    rb_define_method(rb_cInstruction_PUTNIL, "to_i", insn_PUTNIL_to_i, 0);
    rb_define_method(rb_cInstruction_PUTNIL, "operand_types", insn_PUTNIL_operand_types, 0);
    rb_define_method(rb_cInstruction_PUTNIL, "operand_names", insn_PUTNIL_operand_names, 0);
    rb_define_method(rb_cInstruction_PUTNIL, "name", insn_PUTNIL_name, 0);
  }
  {
    VALUE rb_cInstruction_PUTSELF = instruction_class[16] = rb_define_class_under(rb_cInstruction, "PUTSELF", rb_cInstruction);
    rb_define_method(rb_cInstruction_PUTSELF, "length", insn_PUTSELF_length, 0);
    rb_define_method(rb_cInstruction_PUTSELF, "to_i", insn_PUTSELF_to_i, 0);
    rb_define_method(rb_cInstruction_PUTSELF, "operand_types", insn_PUTSELF_operand_types, 0);
    rb_define_method(rb_cInstruction_PUTSELF, "operand_names", insn_PUTSELF_operand_names, 0);
    rb_define_method(rb_cInstruction_PUTSELF, "name", insn_PUTSELF_name, 0);
  }
  {
    VALUE rb_cInstruction_PUTOBJECT = instruction_class[17] = rb_define_class_under(rb_cInstruction, "PUTOBJECT", rb_cInstruction);
    rb_define_method(rb_cInstruction_PUTOBJECT, "length", insn_PUTOBJECT_length, 0);
    rb_define_method(rb_cInstruction_PUTOBJECT, "to_i", insn_PUTOBJECT_to_i, 0);
    rb_define_method(rb_cInstruction_PUTOBJECT, "operand_types", insn_PUTOBJECT_operand_types, 0);
    rb_define_method(rb_cInstruction_PUTOBJECT, "operand_names", insn_PUTOBJECT_operand_names, 0);
    rb_define_method(rb_cInstruction_PUTOBJECT, "name", insn_PUTOBJECT_name, 0);
  }
  {
    VALUE rb_cInstruction_PUTSPECIALOBJECT = instruction_class[18] = rb_define_class_under(rb_cInstruction, "PUTSPECIALOBJECT", rb_cInstruction);
    rb_define_method(rb_cInstruction_PUTSPECIALOBJECT, "length", insn_PUTSPECIALOBJECT_length, 0);
    rb_define_method(rb_cInstruction_PUTSPECIALOBJECT, "to_i", insn_PUTSPECIALOBJECT_to_i, 0);
    rb_define_method(rb_cInstruction_PUTSPECIALOBJECT, "operand_types", insn_PUTSPECIALOBJECT_operand_types, 0);
    rb_define_method(rb_cInstruction_PUTSPECIALOBJECT, "operand_names", insn_PUTSPECIALOBJECT_operand_names, 0);
    rb_define_method(rb_cInstruction_PUTSPECIALOBJECT, "name", insn_PUTSPECIALOBJECT_name, 0);
  }
  {
    VALUE rb_cInstruction_PUTISEQ = instruction_class[19] = rb_define_class_under(rb_cInstruction, "PUTISEQ", rb_cInstruction);
    rb_define_method(rb_cInstruction_PUTISEQ, "length", insn_PUTISEQ_length, 0);
    rb_define_method(rb_cInstruction_PUTISEQ, "to_i", insn_PUTISEQ_to_i, 0);
    rb_define_method(rb_cInstruction_PUTISEQ, "operand_types", insn_PUTISEQ_operand_types, 0);
    rb_define_method(rb_cInstruction_PUTISEQ, "operand_names", insn_PUTISEQ_operand_names, 0);
    rb_define_method(rb_cInstruction_PUTISEQ, "name", insn_PUTISEQ_name, 0);
  }
  {
    VALUE rb_cInstruction_PUTSTRING = instruction_class[20] = rb_define_class_under(rb_cInstruction, "PUTSTRING", rb_cInstruction);
    rb_define_method(rb_cInstruction_PUTSTRING, "length", insn_PUTSTRING_length, 0);
    rb_define_method(rb_cInstruction_PUTSTRING, "to_i", insn_PUTSTRING_to_i, 0);
    rb_define_method(rb_cInstruction_PUTSTRING, "operand_types", insn_PUTSTRING_operand_types, 0);
    rb_define_method(rb_cInstruction_PUTSTRING, "operand_names", insn_PUTSTRING_operand_names, 0);
    rb_define_method(rb_cInstruction_PUTSTRING, "name", insn_PUTSTRING_name, 0);
  }
  {
    VALUE rb_cInstruction_CONCATSTRINGS = instruction_class[21] = rb_define_class_under(rb_cInstruction, "CONCATSTRINGS", rb_cInstruction);
    rb_define_method(rb_cInstruction_CONCATSTRINGS, "length", insn_CONCATSTRINGS_length, 0);
    rb_define_method(rb_cInstruction_CONCATSTRINGS, "to_i", insn_CONCATSTRINGS_to_i, 0);
    rb_define_method(rb_cInstruction_CONCATSTRINGS, "operand_types", insn_CONCATSTRINGS_operand_types, 0);
    rb_define_method(rb_cInstruction_CONCATSTRINGS, "operand_names", insn_CONCATSTRINGS_operand_names, 0);
    rb_define_method(rb_cInstruction_CONCATSTRINGS, "name", insn_CONCATSTRINGS_name, 0);
  }
  {
    VALUE rb_cInstruction_TOSTRING = instruction_class[22] = rb_define_class_under(rb_cInstruction, "TOSTRING", rb_cInstruction);
    rb_define_method(rb_cInstruction_TOSTRING, "length", insn_TOSTRING_length, 0);
    rb_define_method(rb_cInstruction_TOSTRING, "to_i", insn_TOSTRING_to_i, 0);
    rb_define_method(rb_cInstruction_TOSTRING, "operand_types", insn_TOSTRING_operand_types, 0);
    rb_define_method(rb_cInstruction_TOSTRING, "operand_names", insn_TOSTRING_operand_names, 0);
    rb_define_method(rb_cInstruction_TOSTRING, "name", insn_TOSTRING_name, 0);
  }
  {
    VALUE rb_cInstruction_TOREGEXP = instruction_class[23] = rb_define_class_under(rb_cInstruction, "TOREGEXP", rb_cInstruction);
    rb_define_method(rb_cInstruction_TOREGEXP, "length", insn_TOREGEXP_length, 0);
    rb_define_method(rb_cInstruction_TOREGEXP, "to_i", insn_TOREGEXP_to_i, 0);
    rb_define_method(rb_cInstruction_TOREGEXP, "operand_types", insn_TOREGEXP_operand_types, 0);
    rb_define_method(rb_cInstruction_TOREGEXP, "operand_names", insn_TOREGEXP_operand_names, 0);
    rb_define_method(rb_cInstruction_TOREGEXP, "name", insn_TOREGEXP_name, 0);
  }
  {
    VALUE rb_cInstruction_NEWARRAY = instruction_class[24] = rb_define_class_under(rb_cInstruction, "NEWARRAY", rb_cInstruction);
    rb_define_method(rb_cInstruction_NEWARRAY, "length", insn_NEWARRAY_length, 0);
    rb_define_method(rb_cInstruction_NEWARRAY, "to_i", insn_NEWARRAY_to_i, 0);
    rb_define_method(rb_cInstruction_NEWARRAY, "operand_types", insn_NEWARRAY_operand_types, 0);
    rb_define_method(rb_cInstruction_NEWARRAY, "operand_names", insn_NEWARRAY_operand_names, 0);
    rb_define_method(rb_cInstruction_NEWARRAY, "name", insn_NEWARRAY_name, 0);
  }
  {
    VALUE rb_cInstruction_DUPARRAY = instruction_class[25] = rb_define_class_under(rb_cInstruction, "DUPARRAY", rb_cInstruction);
    rb_define_method(rb_cInstruction_DUPARRAY, "length", insn_DUPARRAY_length, 0);
    rb_define_method(rb_cInstruction_DUPARRAY, "to_i", insn_DUPARRAY_to_i, 0);
    rb_define_method(rb_cInstruction_DUPARRAY, "operand_types", insn_DUPARRAY_operand_types, 0);
    rb_define_method(rb_cInstruction_DUPARRAY, "operand_names", insn_DUPARRAY_operand_names, 0);
    rb_define_method(rb_cInstruction_DUPARRAY, "name", insn_DUPARRAY_name, 0);
  }
  {
    VALUE rb_cInstruction_EXPANDARRAY = instruction_class[26] = rb_define_class_under(rb_cInstruction, "EXPANDARRAY", rb_cInstruction);
    rb_define_method(rb_cInstruction_EXPANDARRAY, "length", insn_EXPANDARRAY_length, 0);
    rb_define_method(rb_cInstruction_EXPANDARRAY, "to_i", insn_EXPANDARRAY_to_i, 0);
    rb_define_method(rb_cInstruction_EXPANDARRAY, "operand_types", insn_EXPANDARRAY_operand_types, 0);
    rb_define_method(rb_cInstruction_EXPANDARRAY, "operand_names", insn_EXPANDARRAY_operand_names, 0);
    rb_define_method(rb_cInstruction_EXPANDARRAY, "name", insn_EXPANDARRAY_name, 0);
  }
  {
    VALUE rb_cInstruction_CONCATARRAY = instruction_class[27] = rb_define_class_under(rb_cInstruction, "CONCATARRAY", rb_cInstruction);
    rb_define_method(rb_cInstruction_CONCATARRAY, "length", insn_CONCATARRAY_length, 0);
    rb_define_method(rb_cInstruction_CONCATARRAY, "to_i", insn_CONCATARRAY_to_i, 0);
    rb_define_method(rb_cInstruction_CONCATARRAY, "operand_types", insn_CONCATARRAY_operand_types, 0);
    rb_define_method(rb_cInstruction_CONCATARRAY, "operand_names", insn_CONCATARRAY_operand_names, 0);
    rb_define_method(rb_cInstruction_CONCATARRAY, "name", insn_CONCATARRAY_name, 0);
  }
  {
    VALUE rb_cInstruction_SPLATARRAY = instruction_class[28] = rb_define_class_under(rb_cInstruction, "SPLATARRAY", rb_cInstruction);
    rb_define_method(rb_cInstruction_SPLATARRAY, "length", insn_SPLATARRAY_length, 0);
    rb_define_method(rb_cInstruction_SPLATARRAY, "to_i", insn_SPLATARRAY_to_i, 0);
    rb_define_method(rb_cInstruction_SPLATARRAY, "operand_types", insn_SPLATARRAY_operand_types, 0);
    rb_define_method(rb_cInstruction_SPLATARRAY, "operand_names", insn_SPLATARRAY_operand_names, 0);
    rb_define_method(rb_cInstruction_SPLATARRAY, "name", insn_SPLATARRAY_name, 0);
  }
  {
    VALUE rb_cInstruction_CHECKINCLUDEARRAY = instruction_class[29] = rb_define_class_under(rb_cInstruction, "CHECKINCLUDEARRAY", rb_cInstruction);
    rb_define_method(rb_cInstruction_CHECKINCLUDEARRAY, "length", insn_CHECKINCLUDEARRAY_length, 0);
    rb_define_method(rb_cInstruction_CHECKINCLUDEARRAY, "to_i", insn_CHECKINCLUDEARRAY_to_i, 0);
    rb_define_method(rb_cInstruction_CHECKINCLUDEARRAY, "operand_types", insn_CHECKINCLUDEARRAY_operand_types, 0);
    rb_define_method(rb_cInstruction_CHECKINCLUDEARRAY, "operand_names", insn_CHECKINCLUDEARRAY_operand_names, 0);
    rb_define_method(rb_cInstruction_CHECKINCLUDEARRAY, "name", insn_CHECKINCLUDEARRAY_name, 0);
  }
  {
    VALUE rb_cInstruction_NEWHASH = instruction_class[30] = rb_define_class_under(rb_cInstruction, "NEWHASH", rb_cInstruction);
    rb_define_method(rb_cInstruction_NEWHASH, "length", insn_NEWHASH_length, 0);
    rb_define_method(rb_cInstruction_NEWHASH, "to_i", insn_NEWHASH_to_i, 0);
    rb_define_method(rb_cInstruction_NEWHASH, "operand_types", insn_NEWHASH_operand_types, 0);
    rb_define_method(rb_cInstruction_NEWHASH, "operand_names", insn_NEWHASH_operand_names, 0);
    rb_define_method(rb_cInstruction_NEWHASH, "name", insn_NEWHASH_name, 0);
  }
  {
    VALUE rb_cInstruction_NEWRANGE = instruction_class[31] = rb_define_class_under(rb_cInstruction, "NEWRANGE", rb_cInstruction);
    rb_define_method(rb_cInstruction_NEWRANGE, "length", insn_NEWRANGE_length, 0);
    rb_define_method(rb_cInstruction_NEWRANGE, "to_i", insn_NEWRANGE_to_i, 0);
    rb_define_method(rb_cInstruction_NEWRANGE, "operand_types", insn_NEWRANGE_operand_types, 0);
    rb_define_method(rb_cInstruction_NEWRANGE, "operand_names", insn_NEWRANGE_operand_names, 0);
    rb_define_method(rb_cInstruction_NEWRANGE, "name", insn_NEWRANGE_name, 0);
  }
  {
    VALUE rb_cInstruction_POP = instruction_class[32] = rb_define_class_under(rb_cInstruction, "POP", rb_cInstruction);
    rb_define_method(rb_cInstruction_POP, "length", insn_POP_length, 0);
    rb_define_method(rb_cInstruction_POP, "to_i", insn_POP_to_i, 0);
    rb_define_method(rb_cInstruction_POP, "operand_types", insn_POP_operand_types, 0);
    rb_define_method(rb_cInstruction_POP, "operand_names", insn_POP_operand_names, 0);
    rb_define_method(rb_cInstruction_POP, "name", insn_POP_name, 0);
  }
  {
    VALUE rb_cInstruction_DUP = instruction_class[33] = rb_define_class_under(rb_cInstruction, "DUP", rb_cInstruction);
    rb_define_method(rb_cInstruction_DUP, "length", insn_DUP_length, 0);
    rb_define_method(rb_cInstruction_DUP, "to_i", insn_DUP_to_i, 0);
    rb_define_method(rb_cInstruction_DUP, "operand_types", insn_DUP_operand_types, 0);
    rb_define_method(rb_cInstruction_DUP, "operand_names", insn_DUP_operand_names, 0);
    rb_define_method(rb_cInstruction_DUP, "name", insn_DUP_name, 0);
  }
  {
    VALUE rb_cInstruction_DUPN = instruction_class[34] = rb_define_class_under(rb_cInstruction, "DUPN", rb_cInstruction);
    rb_define_method(rb_cInstruction_DUPN, "length", insn_DUPN_length, 0);
    rb_define_method(rb_cInstruction_DUPN, "to_i", insn_DUPN_to_i, 0);
    rb_define_method(rb_cInstruction_DUPN, "operand_types", insn_DUPN_operand_types, 0);
    rb_define_method(rb_cInstruction_DUPN, "operand_names", insn_DUPN_operand_names, 0);
    rb_define_method(rb_cInstruction_DUPN, "name", insn_DUPN_name, 0);
  }
  {
    VALUE rb_cInstruction_SWAP = instruction_class[35] = rb_define_class_under(rb_cInstruction, "SWAP", rb_cInstruction);
    rb_define_method(rb_cInstruction_SWAP, "length", insn_SWAP_length, 0);
    rb_define_method(rb_cInstruction_SWAP, "to_i", insn_SWAP_to_i, 0);
    rb_define_method(rb_cInstruction_SWAP, "operand_types", insn_SWAP_operand_types, 0);
    rb_define_method(rb_cInstruction_SWAP, "operand_names", insn_SWAP_operand_names, 0);
    rb_define_method(rb_cInstruction_SWAP, "name", insn_SWAP_name, 0);
  }
  {
    VALUE rb_cInstruction_REPUT = instruction_class[36] = rb_define_class_under(rb_cInstruction, "REPUT", rb_cInstruction);
    rb_define_method(rb_cInstruction_REPUT, "length", insn_REPUT_length, 0);
    rb_define_method(rb_cInstruction_REPUT, "to_i", insn_REPUT_to_i, 0);
    rb_define_method(rb_cInstruction_REPUT, "operand_types", insn_REPUT_operand_types, 0);
    rb_define_method(rb_cInstruction_REPUT, "operand_names", insn_REPUT_operand_names, 0);
    rb_define_method(rb_cInstruction_REPUT, "name", insn_REPUT_name, 0);
  }
  {
    VALUE rb_cInstruction_TOPN = instruction_class[37] = rb_define_class_under(rb_cInstruction, "TOPN", rb_cInstruction);
    rb_define_method(rb_cInstruction_TOPN, "length", insn_TOPN_length, 0);
    rb_define_method(rb_cInstruction_TOPN, "to_i", insn_TOPN_to_i, 0);
    rb_define_method(rb_cInstruction_TOPN, "operand_types", insn_TOPN_operand_types, 0);
    rb_define_method(rb_cInstruction_TOPN, "operand_names", insn_TOPN_operand_names, 0);
    rb_define_method(rb_cInstruction_TOPN, "name", insn_TOPN_name, 0);
  }
  {
    VALUE rb_cInstruction_SETN = instruction_class[38] = rb_define_class_under(rb_cInstruction, "SETN", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETN, "length", insn_SETN_length, 0);
    rb_define_method(rb_cInstruction_SETN, "to_i", insn_SETN_to_i, 0);
    rb_define_method(rb_cInstruction_SETN, "operand_types", insn_SETN_operand_types, 0);
    rb_define_method(rb_cInstruction_SETN, "operand_names", insn_SETN_operand_names, 0);
    rb_define_method(rb_cInstruction_SETN, "name", insn_SETN_name, 0);
  }
  {
    VALUE rb_cInstruction_ADJUSTSTACK = instruction_class[39] = rb_define_class_under(rb_cInstruction, "ADJUSTSTACK", rb_cInstruction);
    rb_define_method(rb_cInstruction_ADJUSTSTACK, "length", insn_ADJUSTSTACK_length, 0);
    rb_define_method(rb_cInstruction_ADJUSTSTACK, "to_i", insn_ADJUSTSTACK_to_i, 0);
    rb_define_method(rb_cInstruction_ADJUSTSTACK, "operand_types", insn_ADJUSTSTACK_operand_types, 0);
    rb_define_method(rb_cInstruction_ADJUSTSTACK, "operand_names", insn_ADJUSTSTACK_operand_names, 0);
    rb_define_method(rb_cInstruction_ADJUSTSTACK, "name", insn_ADJUSTSTACK_name, 0);
  }
  {
    VALUE rb_cInstruction_DEFINED = instruction_class[40] = rb_define_class_under(rb_cInstruction, "DEFINED", rb_cInstruction);
    rb_define_method(rb_cInstruction_DEFINED, "length", insn_DEFINED_length, 0);
    rb_define_method(rb_cInstruction_DEFINED, "to_i", insn_DEFINED_to_i, 0);
    rb_define_method(rb_cInstruction_DEFINED, "operand_types", insn_DEFINED_operand_types, 0);
    rb_define_method(rb_cInstruction_DEFINED, "operand_names", insn_DEFINED_operand_names, 0);
    rb_define_method(rb_cInstruction_DEFINED, "name", insn_DEFINED_name, 0);
  }
  {
    VALUE rb_cInstruction_TRACE = instruction_class[41] = rb_define_class_under(rb_cInstruction, "TRACE", rb_cInstruction);
    rb_define_method(rb_cInstruction_TRACE, "length", insn_TRACE_length, 0);
    rb_define_method(rb_cInstruction_TRACE, "to_i", insn_TRACE_to_i, 0);
    rb_define_method(rb_cInstruction_TRACE, "operand_types", insn_TRACE_operand_types, 0);
    rb_define_method(rb_cInstruction_TRACE, "operand_names", insn_TRACE_operand_names, 0);
    rb_define_method(rb_cInstruction_TRACE, "name", insn_TRACE_name, 0);
  }
  {
    VALUE rb_cInstruction_DEFINECLASS = instruction_class[42] = rb_define_class_under(rb_cInstruction, "DEFINECLASS", rb_cInstruction);
    rb_define_method(rb_cInstruction_DEFINECLASS, "length", insn_DEFINECLASS_length, 0);
    rb_define_method(rb_cInstruction_DEFINECLASS, "to_i", insn_DEFINECLASS_to_i, 0);
    rb_define_method(rb_cInstruction_DEFINECLASS, "operand_types", insn_DEFINECLASS_operand_types, 0);
    rb_define_method(rb_cInstruction_DEFINECLASS, "operand_names", insn_DEFINECLASS_operand_names, 0);
    rb_define_method(rb_cInstruction_DEFINECLASS, "name", insn_DEFINECLASS_name, 0);
  }
  {
    VALUE rb_cInstruction_SEND = instruction_class[43] = rb_define_class_under(rb_cInstruction, "SEND", rb_cInstruction);
    rb_define_method(rb_cInstruction_SEND, "length", insn_SEND_length, 0);
    rb_define_method(rb_cInstruction_SEND, "to_i", insn_SEND_to_i, 0);
    rb_define_method(rb_cInstruction_SEND, "operand_types", insn_SEND_operand_types, 0);
    rb_define_method(rb_cInstruction_SEND, "operand_names", insn_SEND_operand_names, 0);
    rb_define_method(rb_cInstruction_SEND, "name", insn_SEND_name, 0);
  }
  {
    VALUE rb_cInstruction_INVOKESUPER = instruction_class[44] = rb_define_class_under(rb_cInstruction, "INVOKESUPER", rb_cInstruction);
    rb_define_method(rb_cInstruction_INVOKESUPER, "length", insn_INVOKESUPER_length, 0);
    rb_define_method(rb_cInstruction_INVOKESUPER, "to_i", insn_INVOKESUPER_to_i, 0);
    rb_define_method(rb_cInstruction_INVOKESUPER, "operand_types", insn_INVOKESUPER_operand_types, 0);
    rb_define_method(rb_cInstruction_INVOKESUPER, "operand_names", insn_INVOKESUPER_operand_names, 0);
    rb_define_method(rb_cInstruction_INVOKESUPER, "name", insn_INVOKESUPER_name, 0);
  }
  {
    VALUE rb_cInstruction_INVOKEBLOCK = instruction_class[45] = rb_define_class_under(rb_cInstruction, "INVOKEBLOCK", rb_cInstruction);
    rb_define_method(rb_cInstruction_INVOKEBLOCK, "length", insn_INVOKEBLOCK_length, 0);
    rb_define_method(rb_cInstruction_INVOKEBLOCK, "to_i", insn_INVOKEBLOCK_to_i, 0);
    rb_define_method(rb_cInstruction_INVOKEBLOCK, "operand_types", insn_INVOKEBLOCK_operand_types, 0);
    rb_define_method(rb_cInstruction_INVOKEBLOCK, "operand_names", insn_INVOKEBLOCK_operand_names, 0);
    rb_define_method(rb_cInstruction_INVOKEBLOCK, "name", insn_INVOKEBLOCK_name, 0);
  }
  {
    VALUE rb_cInstruction_LEAVE = instruction_class[46] = rb_define_class_under(rb_cInstruction, "LEAVE", rb_cInstruction);
    rb_define_method(rb_cInstruction_LEAVE, "length", insn_LEAVE_length, 0);
    rb_define_method(rb_cInstruction_LEAVE, "to_i", insn_LEAVE_to_i, 0);
    rb_define_method(rb_cInstruction_LEAVE, "operand_types", insn_LEAVE_operand_types, 0);
    rb_define_method(rb_cInstruction_LEAVE, "operand_names", insn_LEAVE_operand_names, 0);
    rb_define_method(rb_cInstruction_LEAVE, "name", insn_LEAVE_name, 0);
  }
  {
    VALUE rb_cInstruction_FINISH = instruction_class[47] = rb_define_class_under(rb_cInstruction, "FINISH", rb_cInstruction);
    rb_define_method(rb_cInstruction_FINISH, "length", insn_FINISH_length, 0);
    rb_define_method(rb_cInstruction_FINISH, "to_i", insn_FINISH_to_i, 0);
    rb_define_method(rb_cInstruction_FINISH, "operand_types", insn_FINISH_operand_types, 0);
    rb_define_method(rb_cInstruction_FINISH, "operand_names", insn_FINISH_operand_names, 0);
    rb_define_method(rb_cInstruction_FINISH, "name", insn_FINISH_name, 0);
  }
  {
    VALUE rb_cInstruction_THROW = instruction_class[48] = rb_define_class_under(rb_cInstruction, "THROW", rb_cInstruction);
    rb_define_method(rb_cInstruction_THROW, "length", insn_THROW_length, 0);
    rb_define_method(rb_cInstruction_THROW, "to_i", insn_THROW_to_i, 0);
    rb_define_method(rb_cInstruction_THROW, "operand_types", insn_THROW_operand_types, 0);
    rb_define_method(rb_cInstruction_THROW, "operand_names", insn_THROW_operand_names, 0);
    rb_define_method(rb_cInstruction_THROW, "name", insn_THROW_name, 0);
  }
  {
    VALUE rb_cInstruction_JUMP = instruction_class[49] = rb_define_class_under(rb_cInstruction, "JUMP", rb_cInstruction);
    rb_define_method(rb_cInstruction_JUMP, "length", insn_JUMP_length, 0);
    rb_define_method(rb_cInstruction_JUMP, "to_i", insn_JUMP_to_i, 0);
    rb_define_method(rb_cInstruction_JUMP, "operand_types", insn_JUMP_operand_types, 0);
    rb_define_method(rb_cInstruction_JUMP, "operand_names", insn_JUMP_operand_names, 0);
    rb_define_method(rb_cInstruction_JUMP, "name", insn_JUMP_name, 0);
  }
  {
    VALUE rb_cInstruction_BRANCHIF = instruction_class[50] = rb_define_class_under(rb_cInstruction, "BRANCHIF", rb_cInstruction);
    rb_define_method(rb_cInstruction_BRANCHIF, "length", insn_BRANCHIF_length, 0);
    rb_define_method(rb_cInstruction_BRANCHIF, "to_i", insn_BRANCHIF_to_i, 0);
    rb_define_method(rb_cInstruction_BRANCHIF, "operand_types", insn_BRANCHIF_operand_types, 0);
    rb_define_method(rb_cInstruction_BRANCHIF, "operand_names", insn_BRANCHIF_operand_names, 0);
    rb_define_method(rb_cInstruction_BRANCHIF, "name", insn_BRANCHIF_name, 0);
  }
  {
    VALUE rb_cInstruction_BRANCHUNLESS = instruction_class[51] = rb_define_class_under(rb_cInstruction, "BRANCHUNLESS", rb_cInstruction);
    rb_define_method(rb_cInstruction_BRANCHUNLESS, "length", insn_BRANCHUNLESS_length, 0);
    rb_define_method(rb_cInstruction_BRANCHUNLESS, "to_i", insn_BRANCHUNLESS_to_i, 0);
    rb_define_method(rb_cInstruction_BRANCHUNLESS, "operand_types", insn_BRANCHUNLESS_operand_types, 0);
    rb_define_method(rb_cInstruction_BRANCHUNLESS, "operand_names", insn_BRANCHUNLESS_operand_names, 0);
    rb_define_method(rb_cInstruction_BRANCHUNLESS, "name", insn_BRANCHUNLESS_name, 0);
  }
  {
    VALUE rb_cInstruction_GETINLINECACHE = instruction_class[52] = rb_define_class_under(rb_cInstruction, "GETINLINECACHE", rb_cInstruction);
    rb_define_method(rb_cInstruction_GETINLINECACHE, "length", insn_GETINLINECACHE_length, 0);
    rb_define_method(rb_cInstruction_GETINLINECACHE, "to_i", insn_GETINLINECACHE_to_i, 0);
    rb_define_method(rb_cInstruction_GETINLINECACHE, "operand_types", insn_GETINLINECACHE_operand_types, 0);
    rb_define_method(rb_cInstruction_GETINLINECACHE, "operand_names", insn_GETINLINECACHE_operand_names, 0);
    rb_define_method(rb_cInstruction_GETINLINECACHE, "name", insn_GETINLINECACHE_name, 0);
  }
  {
    VALUE rb_cInstruction_ONCEINLINECACHE = instruction_class[53] = rb_define_class_under(rb_cInstruction, "ONCEINLINECACHE", rb_cInstruction);
    rb_define_method(rb_cInstruction_ONCEINLINECACHE, "length", insn_ONCEINLINECACHE_length, 0);
    rb_define_method(rb_cInstruction_ONCEINLINECACHE, "to_i", insn_ONCEINLINECACHE_to_i, 0);
    rb_define_method(rb_cInstruction_ONCEINLINECACHE, "operand_types", insn_ONCEINLINECACHE_operand_types, 0);
    rb_define_method(rb_cInstruction_ONCEINLINECACHE, "operand_names", insn_ONCEINLINECACHE_operand_names, 0);
    rb_define_method(rb_cInstruction_ONCEINLINECACHE, "name", insn_ONCEINLINECACHE_name, 0);
  }
  {
    VALUE rb_cInstruction_SETINLINECACHE = instruction_class[54] = rb_define_class_under(rb_cInstruction, "SETINLINECACHE", rb_cInstruction);
    rb_define_method(rb_cInstruction_SETINLINECACHE, "length", insn_SETINLINECACHE_length, 0);
    rb_define_method(rb_cInstruction_SETINLINECACHE, "to_i", insn_SETINLINECACHE_to_i, 0);
    rb_define_method(rb_cInstruction_SETINLINECACHE, "operand_types", insn_SETINLINECACHE_operand_types, 0);
    rb_define_method(rb_cInstruction_SETINLINECACHE, "operand_names", insn_SETINLINECACHE_operand_names, 0);
    rb_define_method(rb_cInstruction_SETINLINECACHE, "name", insn_SETINLINECACHE_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_CASE_DISPATCH = instruction_class[55] = rb_define_class_under(rb_cInstruction, "OPT_CASE_DISPATCH", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_CASE_DISPATCH, "length", insn_OPT_CASE_DISPATCH_length, 0);
    rb_define_method(rb_cInstruction_OPT_CASE_DISPATCH, "to_i", insn_OPT_CASE_DISPATCH_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_CASE_DISPATCH, "operand_types", insn_OPT_CASE_DISPATCH_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_CASE_DISPATCH, "operand_names", insn_OPT_CASE_DISPATCH_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_CASE_DISPATCH, "name", insn_OPT_CASE_DISPATCH_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_CHECKENV = instruction_class[56] = rb_define_class_under(rb_cInstruction, "OPT_CHECKENV", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_CHECKENV, "length", insn_OPT_CHECKENV_length, 0);
    rb_define_method(rb_cInstruction_OPT_CHECKENV, "to_i", insn_OPT_CHECKENV_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_CHECKENV, "operand_types", insn_OPT_CHECKENV_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_CHECKENV, "operand_names", insn_OPT_CHECKENV_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_CHECKENV, "name", insn_OPT_CHECKENV_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_PLUS = instruction_class[57] = rb_define_class_under(rb_cInstruction, "OPT_PLUS", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_PLUS, "length", insn_OPT_PLUS_length, 0);
    rb_define_method(rb_cInstruction_OPT_PLUS, "to_i", insn_OPT_PLUS_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_PLUS, "operand_types", insn_OPT_PLUS_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_PLUS, "operand_names", insn_OPT_PLUS_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_PLUS, "name", insn_OPT_PLUS_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_MINUS = instruction_class[58] = rb_define_class_under(rb_cInstruction, "OPT_MINUS", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_MINUS, "length", insn_OPT_MINUS_length, 0);
    rb_define_method(rb_cInstruction_OPT_MINUS, "to_i", insn_OPT_MINUS_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_MINUS, "operand_types", insn_OPT_MINUS_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_MINUS, "operand_names", insn_OPT_MINUS_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_MINUS, "name", insn_OPT_MINUS_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_MULT = instruction_class[59] = rb_define_class_under(rb_cInstruction, "OPT_MULT", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_MULT, "length", insn_OPT_MULT_length, 0);
    rb_define_method(rb_cInstruction_OPT_MULT, "to_i", insn_OPT_MULT_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_MULT, "operand_types", insn_OPT_MULT_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_MULT, "operand_names", insn_OPT_MULT_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_MULT, "name", insn_OPT_MULT_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_DIV = instruction_class[60] = rb_define_class_under(rb_cInstruction, "OPT_DIV", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_DIV, "length", insn_OPT_DIV_length, 0);
    rb_define_method(rb_cInstruction_OPT_DIV, "to_i", insn_OPT_DIV_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_DIV, "operand_types", insn_OPT_DIV_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_DIV, "operand_names", insn_OPT_DIV_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_DIV, "name", insn_OPT_DIV_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_MOD = instruction_class[61] = rb_define_class_under(rb_cInstruction, "OPT_MOD", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_MOD, "length", insn_OPT_MOD_length, 0);
    rb_define_method(rb_cInstruction_OPT_MOD, "to_i", insn_OPT_MOD_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_MOD, "operand_types", insn_OPT_MOD_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_MOD, "operand_names", insn_OPT_MOD_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_MOD, "name", insn_OPT_MOD_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_EQ = instruction_class[62] = rb_define_class_under(rb_cInstruction, "OPT_EQ", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_EQ, "length", insn_OPT_EQ_length, 0);
    rb_define_method(rb_cInstruction_OPT_EQ, "to_i", insn_OPT_EQ_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_EQ, "operand_types", insn_OPT_EQ_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_EQ, "operand_names", insn_OPT_EQ_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_EQ, "name", insn_OPT_EQ_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_NEQ = instruction_class[63] = rb_define_class_under(rb_cInstruction, "OPT_NEQ", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_NEQ, "length", insn_OPT_NEQ_length, 0);
    rb_define_method(rb_cInstruction_OPT_NEQ, "to_i", insn_OPT_NEQ_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_NEQ, "operand_types", insn_OPT_NEQ_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_NEQ, "operand_names", insn_OPT_NEQ_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_NEQ, "name", insn_OPT_NEQ_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_LT = instruction_class[64] = rb_define_class_under(rb_cInstruction, "OPT_LT", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_LT, "length", insn_OPT_LT_length, 0);
    rb_define_method(rb_cInstruction_OPT_LT, "to_i", insn_OPT_LT_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_LT, "operand_types", insn_OPT_LT_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_LT, "operand_names", insn_OPT_LT_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_LT, "name", insn_OPT_LT_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_LE = instruction_class[65] = rb_define_class_under(rb_cInstruction, "OPT_LE", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_LE, "length", insn_OPT_LE_length, 0);
    rb_define_method(rb_cInstruction_OPT_LE, "to_i", insn_OPT_LE_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_LE, "operand_types", insn_OPT_LE_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_LE, "operand_names", insn_OPT_LE_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_LE, "name", insn_OPT_LE_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_GT = instruction_class[66] = rb_define_class_under(rb_cInstruction, "OPT_GT", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_GT, "length", insn_OPT_GT_length, 0);
    rb_define_method(rb_cInstruction_OPT_GT, "to_i", insn_OPT_GT_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_GT, "operand_types", insn_OPT_GT_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_GT, "operand_names", insn_OPT_GT_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_GT, "name", insn_OPT_GT_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_GE = instruction_class[67] = rb_define_class_under(rb_cInstruction, "OPT_GE", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_GE, "length", insn_OPT_GE_length, 0);
    rb_define_method(rb_cInstruction_OPT_GE, "to_i", insn_OPT_GE_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_GE, "operand_types", insn_OPT_GE_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_GE, "operand_names", insn_OPT_GE_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_GE, "name", insn_OPT_GE_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_LTLT = instruction_class[68] = rb_define_class_under(rb_cInstruction, "OPT_LTLT", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_LTLT, "length", insn_OPT_LTLT_length, 0);
    rb_define_method(rb_cInstruction_OPT_LTLT, "to_i", insn_OPT_LTLT_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_LTLT, "operand_types", insn_OPT_LTLT_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_LTLT, "operand_names", insn_OPT_LTLT_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_LTLT, "name", insn_OPT_LTLT_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_AREF = instruction_class[69] = rb_define_class_under(rb_cInstruction, "OPT_AREF", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_AREF, "length", insn_OPT_AREF_length, 0);
    rb_define_method(rb_cInstruction_OPT_AREF, "to_i", insn_OPT_AREF_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_AREF, "operand_types", insn_OPT_AREF_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_AREF, "operand_names", insn_OPT_AREF_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_AREF, "name", insn_OPT_AREF_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_ASET = instruction_class[70] = rb_define_class_under(rb_cInstruction, "OPT_ASET", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_ASET, "length", insn_OPT_ASET_length, 0);
    rb_define_method(rb_cInstruction_OPT_ASET, "to_i", insn_OPT_ASET_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_ASET, "operand_types", insn_OPT_ASET_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_ASET, "operand_names", insn_OPT_ASET_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_ASET, "name", insn_OPT_ASET_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_LENGTH = instruction_class[71] = rb_define_class_under(rb_cInstruction, "OPT_LENGTH", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_LENGTH, "length", insn_OPT_LENGTH_length, 0);
    rb_define_method(rb_cInstruction_OPT_LENGTH, "to_i", insn_OPT_LENGTH_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_LENGTH, "operand_types", insn_OPT_LENGTH_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_LENGTH, "operand_names", insn_OPT_LENGTH_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_LENGTH, "name", insn_OPT_LENGTH_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_SUCC = instruction_class[72] = rb_define_class_under(rb_cInstruction, "OPT_SUCC", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_SUCC, "length", insn_OPT_SUCC_length, 0);
    rb_define_method(rb_cInstruction_OPT_SUCC, "to_i", insn_OPT_SUCC_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_SUCC, "operand_types", insn_OPT_SUCC_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_SUCC, "operand_names", insn_OPT_SUCC_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_SUCC, "name", insn_OPT_SUCC_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_NOT = instruction_class[73] = rb_define_class_under(rb_cInstruction, "OPT_NOT", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_NOT, "length", insn_OPT_NOT_length, 0);
    rb_define_method(rb_cInstruction_OPT_NOT, "to_i", insn_OPT_NOT_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_NOT, "operand_types", insn_OPT_NOT_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_NOT, "operand_names", insn_OPT_NOT_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_NOT, "name", insn_OPT_NOT_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_REGEXPMATCH1 = instruction_class[74] = rb_define_class_under(rb_cInstruction, "OPT_REGEXPMATCH1", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH1, "length", insn_OPT_REGEXPMATCH1_length, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH1, "to_i", insn_OPT_REGEXPMATCH1_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH1, "operand_types", insn_OPT_REGEXPMATCH1_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH1, "operand_names", insn_OPT_REGEXPMATCH1_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH1, "name", insn_OPT_REGEXPMATCH1_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_REGEXPMATCH2 = instruction_class[75] = rb_define_class_under(rb_cInstruction, "OPT_REGEXPMATCH2", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH2, "length", insn_OPT_REGEXPMATCH2_length, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH2, "to_i", insn_OPT_REGEXPMATCH2_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH2, "operand_types", insn_OPT_REGEXPMATCH2_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH2, "operand_names", insn_OPT_REGEXPMATCH2_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_REGEXPMATCH2, "name", insn_OPT_REGEXPMATCH2_name, 0);
  }
  {
    VALUE rb_cInstruction_OPT_CALL_C_FUNCTION = instruction_class[76] = rb_define_class_under(rb_cInstruction, "OPT_CALL_C_FUNCTION", rb_cInstruction);
    rb_define_method(rb_cInstruction_OPT_CALL_C_FUNCTION, "length", insn_OPT_CALL_C_FUNCTION_length, 0);
    rb_define_method(rb_cInstruction_OPT_CALL_C_FUNCTION, "to_i", insn_OPT_CALL_C_FUNCTION_to_i, 0);
    rb_define_method(rb_cInstruction_OPT_CALL_C_FUNCTION, "operand_types", insn_OPT_CALL_C_FUNCTION_operand_types, 0);
    rb_define_method(rb_cInstruction_OPT_CALL_C_FUNCTION, "operand_names", insn_OPT_CALL_C_FUNCTION_operand_names, 0);
    rb_define_method(rb_cInstruction_OPT_CALL_C_FUNCTION, "name", insn_OPT_CALL_C_FUNCTION_name, 0);
  }
  {
    VALUE rb_cInstruction_BITBLT = instruction_class[77] = rb_define_class_under(rb_cInstruction, "BITBLT", rb_cInstruction);
    rb_define_method(rb_cInstruction_BITBLT, "length", insn_BITBLT_length, 0);
    rb_define_method(rb_cInstruction_BITBLT, "to_i", insn_BITBLT_to_i, 0);
    rb_define_method(rb_cInstruction_BITBLT, "operand_types", insn_BITBLT_operand_types, 0);
    rb_define_method(rb_cInstruction_BITBLT, "operand_names", insn_BITBLT_operand_names, 0);
    rb_define_method(rb_cInstruction_BITBLT, "name", insn_BITBLT_name, 0);
  }
  {
    VALUE rb_cInstruction_ANSWER = instruction_class[78] = rb_define_class_under(rb_cInstruction, "ANSWER", rb_cInstruction);
    rb_define_method(rb_cInstruction_ANSWER, "length", insn_ANSWER_length, 0);
    rb_define_method(rb_cInstruction_ANSWER, "to_i", insn_ANSWER_to_i, 0);
    rb_define_method(rb_cInstruction_ANSWER, "operand_types", insn_ANSWER_operand_types, 0);
    rb_define_method(rb_cInstruction_ANSWER, "operand_names", insn_ANSWER_operand_names, 0);
    rb_define_method(rb_cInstruction_ANSWER, "name", insn_ANSWER_name, 0);
  }
}

#endif

