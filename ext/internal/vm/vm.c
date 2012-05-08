#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

#ifdef HAVE_TYPE_STRUCT_RTYPEDDATA

#  undef GetThreadPtr
#  define GetThreadPtr(obj, ptr) \
   TypedData_Get_Struct((obj), rb_thread_t, p_ruby_threadptr_data_type, (ptr))

  static rb_data_type_t const * p_ruby_threadptr_data_type;

  static void init_ruby_threadptr_data_type()
  {
    VALUE thread = rb_thread_current();
    p_ruby_threadptr_data_type = RTYPEDDATA_TYPE(thread);
  }

#endif

extern rb_vm_t * ruby_current_vm;

/*
 * call-seq:
 *   RubyVM.current => RubyVM
 *
 * Returns the current ruby VM.
 */
static VALUE vm_current(VALUE klass)
{
#ifdef HAVE_TYPE_STRUCT_RTYPEDDATA
  VALUE thread = rb_thread_current();
  rb_thread_t * th;
  ID * local_tbl;
  GetThreadPtr(thread, th);
  return th->vm->self;
#else
  return ruby_current_vm->self;
#endif
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

#ifdef HAVE_TYPE_STRUCT_RTYPEDDATA
  init_ruby_threadptr_data_type();
#endif
}

