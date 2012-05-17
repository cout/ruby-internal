#include "control_frame.h"
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

/* TODO: Using this class is dangerous, because the control frame can be
 * popped but a reference kept around to it.  It will still point to
 * valid memory, but the data itself will be invalid.
 */

static VALUE rb_cVmControlFrame;

void mark_ruby_internal_control_frame(
    struct RubyInternalControlFrame * cfp)
{
  rb_gc_mark(cfp->thread);
}

/*
 * call-seq:
 *   control_frame.iseq => RubyVM::InstructionSequence
 *
 * Return the frame's iseq member.
 */
static VALUE control_frame_iseq(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return (VALUE)cfp->control_frame->iseq;
}

/*
 * call-seq:
 *   control_frame.self => Object
 *
 * Return the frame's self member.
 */
static VALUE control_frame_self(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return cfp->control_frame->self;
}

/*
 * call-seq:
 *   control_frame.block_iseq => RubyVM::InstructionSequence or nil
 *
 * Return the frame's block_iseq member.
 */
static VALUE control_frame_block_iseq(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  if(cfp->control_frame->proc)
  {
    return (VALUE)cfp->control_frame->block_iseq;
  }
  else
  {
    return Qfalse;
  }
}

/*
 * call-seq:
 *   control_frame.proc => ?
 *
 * Return the frame's proc member.
 */
static VALUE control_frame_proc(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);
  return cfp->control_frame->proc;
}

/*
 * call-seq:
 *   control_frame.method_id => Symbol or nil
 *
 * Return the frame's method_id member.
 */
static VALUE control_frame_method_id(VALUE control_frame)
{
  ID method_id;
  struct RubyInternalControlFrame * cfp;

  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);

#if RUBY_VERSION_CODE >= 192
  method_id = cfp->control_frame->me->called_id; /* TODO: right? */
#else
  method_id = cfp->control_frame->method_id;
#endif

  if(method_id)
  {
    return ID2SYM(method_id);
  }
  else
  {
    return Qnil;
  }
}

/*
 * call-seq:
 *   control_frame.method_class => Class
 *
 * Return the frame's method_class member.
 */
static VALUE control_frame_method_class(VALUE control_frame)
{
  VALUE klass;
  struct RubyInternalControlFrame * cfp;

  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);

#if RUBY_VERSION_CODE >= 192
  klass = cfp->control_frame->me->klass; /* TODO: right? */
#else
  klass = cfp->control_frame->method_class;
#endif

  return klass;
}

/*
 * call-seq:
 *   control_frame.prev => RubyVM::ControlFrame
 *
 * Return the frame's prev member.
 */
static VALUE control_frame_prev(VALUE control_frame)
{
  struct RubyInternalControlFrame * cfp;
  rb_thread_t * th;
  rb_control_frame_t * prev_cfp;

  Data_Get_Struct(control_frame, struct RubyInternalControlFrame, cfp);

  GetThreadPtr(cfp->thread, th);

  prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp->control_frame);

  printf("current: %p\n", cfp->control_frame);
  printf("prev: %p\n", prev_cfp);
  printf("th->stack: %p\n", th->stack);
  printf("th->stack + th->stack_size: %p\n", th->stack + th->stack_size);

  if((void *)(th->stack + th->stack_size) == (void *)prev_cfp)
  {
    return Qnil;
  }
  else
  {
    VALUE cfp_v;
    struct RubyInternalControlFrame * new_cfp;

    cfp_v = Data_Make_Struct(
        rb_cVmControlFrame,
        struct RubyInternalControlFrame,
        mark_ruby_internal_control_frame,
        free,
        new_cfp);

    new_cfp->control_frame = prev_cfp;
    new_cfp->thread = cfp->thread;

    return cfp_v;
  }
}

#endif

void Init_control_frame(void)
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
  rb_cVmControlFrame = rb_define_class_under(rb_cRubyVM, "ControlFrame", rb_cObject);
  rb_define_method(rb_cVmControlFrame, "iseq", control_frame_iseq, 0);
  rb_define_method(rb_cVmControlFrame, "self", control_frame_self, 0);
  rb_define_method(rb_cVmControlFrame, "block_iseq", control_frame_block_iseq, 0);
  rb_define_method(rb_cVmControlFrame, "proc", control_frame_proc, 0);
  rb_define_method(rb_cVmControlFrame, "method_id", control_frame_method_id, 0);
  rb_define_method(rb_cVmControlFrame, "method_class", control_frame_method_class, 0);
  rb_define_method(rb_cVmControlFrame, "prev", control_frame_prev, 0);
#endif

#ifdef HAVE_TYPE_STRUCT_RTYPEDDATA
  init_ruby_threadptr_data_type();
#endif
}

