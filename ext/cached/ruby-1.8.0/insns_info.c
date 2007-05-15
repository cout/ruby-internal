#include "insns_info.h"
#include "ruby_version.h"

#ifdef RUBY_HAS_YARV


VALUE instruction_class[YARV_MAX_INSTRUCTION_SIZE];

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


void define_instruction_subclasses(VALUE rb_cInstruction)
{
  /* For rdoc: rb_cVM = rb_define_class("VM", rb_cObject); */
  /* For rdoc: rb_cInstruction = rb_define_class_under(rb_cVM, "Instruction", rb_cObject); */

}

#endif

