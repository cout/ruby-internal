#ifndef insns_info__h_
#define insns_info__h_

#include "ruby.h"

#ifdef RUBY_VM

#include "vm_core.h"

#ifdef HAVE_VM_H
#include "vm.h"
#endif


extern VALUE instruction_class[VM_INSTRUCTION_SIZE];
void define_instruction_subclasses(VALUE rb_cInstruction);

#endif

#endif

