#ifndef insns_info__h_
#define insns_info__h_

#include "ruby_version.h"

#ifdef RUBY_HAS_YARV

#include "ruby.h"
#include "vm_core.h"
#include "vm.h"


extern VALUE instruction_class[VM_INSTRUCTION_SIZE];
void define_instruction_subclasses(VALUE rb_cInstruction);

#endif

#endif

