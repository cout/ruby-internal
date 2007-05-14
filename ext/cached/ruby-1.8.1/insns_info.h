#ifndef insns_info__h_
#define insns_info__h_

#include "vm.h"
#include "ruby.h"
#include "yarvcore.h"


extern VALUE instruction_class[YARV_MAX_INSTRUCTION_SIZE];
void define_instruction_subclasses(VALUE rb_cInstruction);

#endif

