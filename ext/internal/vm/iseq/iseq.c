#include "internal_iseq.h"
#include "insns_info.inc"

#include "internal/node/ruby_internal_node.h"
#include "internal/module/module.h"
#include "internal/vm/instruction/instruction.h"

#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

#ifdef HAVE_ISEQ_H
#include "iseq.h"
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

// Defined but not declared by ruby
VALUE iseq_load(VALUE self, VALUE data, VALUE parent, VALUE opt);

static VALUE rb_cModulePlaceholder;

static VALUE rb_mMarshal;

static VALUE rb_cInlineCache;

static VALUE rb_cCatchTableEntry;

static VALUE marshal_dump(VALUE obj, VALUE limit)
{
  return rb_funcall(rb_mMarshal, rb_intern("dump"), 2, obj, limit);
}

static VALUE marshal_load(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("load"), 1, obj);
}

static rb_iseq_t *
iseq_check(VALUE val)
{
  rb_iseq_t *iseq;
  if(!rb_obj_is_kind_of(val, rb_cISeq))
  {
    rb_raise(
        rb_eTypeError,
        "Expected VM::InstructionSequence, but got %s",
        rb_class2name(CLASS_OF(val)));
  }
  GetISeqPtr(val, iseq);
  if (!iseq->name) {
    rb_raise(rb_eTypeError, "uninitialized InstructionSequence");
  }
  return iseq;
}

/* call-seq:
 *   iseq.self => VM::InstructionSequence
 *
 * Returns the self member of the instruction sequence.
 */
static VALUE iseq_self(VALUE self)
{
  rb_iseq_t * iseqdat = iseq_check(self);
  return iseqdat->self;
}

/* call-seq:
 *   iseq.parent_iseq => VM::InstructionSequence or nil
 *
 * Returns the parent instruction sequence.
 */
static VALUE iseq_parent_iseq(VALUE self)
{
  rb_iseq_t * iseqdat = iseq_check(self);
  rb_iseq_t * parent = iseqdat->parent_iseq;
  if(parent)
  {
    return parent->self;
  }
  else
  {
    return Qnil;
  }
}

/* call-seq:
 *   iseq.local_iseq => VM::InstructionSequence or nil
 *
 * Returns the instruction sequence that contains the local variables.
 */
static VALUE iseq_local_iseq(VALUE self)
{
  rb_iseq_t * iseqdat = iseq_check(self);
  rb_iseq_t * local_iseq = iseqdat->local_iseq;
  if(local_iseq)
  {
    return local_iseq->self;
  }
  else
  {
    return Qnil;
  }
}
/* call-seq:
 *   iseq.name => String
 *
 * Returns the name of the instruction sequence.
 */
static VALUE iseq_name(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return iseqdat->name;
}

/* call-seq:
 *   iseq.filename => String
 *
 * Returns the filename of the instruction sequence.
 */
static VALUE iseq_filename(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return iseqdat->filename;
}

/* call-seq:
 *   iseq.local_table => String
 *
 * Returns the sequence's local table.
 */
static VALUE iseq_local_table(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  VALUE ary = rb_ary_new();
  int j;

  for(j = 0; j < iseqdat->local_table_size; ++j)
  {
    ID id = iseqdat->local_table[j];
    if(rb_id2name(id))
    {
      rb_ary_push(ary, ID2SYM(id));
    }
    else
    {
      // Temporary
      rb_ary_push(ary, Qnil);
    }
  }

  return ary;
}

/* call-seq:
 *   iseq.argc => String
 *
 * Returns the number of non-optional arguments.
 */
static VALUE iseq_argc(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return INT2NUM(iseqdat->argc);
}

/* call-seq:
 *   iseq.arg_simple => true or false
 *
 * Returns true if this sequence takes only simple (non-rest, non-opt,
 * non-block) args.
 */
static VALUE iseq_arg_simple(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return iseqdat->arg_simple ? Qtrue : Qfalse;
}

/* call-seq:
 *   iseq.arg_rest => true or false
 *
 * Returns the index of the rest (*x) arg.
 */
static VALUE iseq_arg_rest(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return INT2NUM(iseqdat->arg_rest);
}

/* call-seq:
 *   iseq.arg_block => true or false
 *
 * Returns the index of the block (&x) arg.
 */
static VALUE iseq_arg_block(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return INT2NUM(iseqdat->arg_block);
}

/* call-seq:
 *   iseq.arg_opt_table => Array of Integer
 *
 * Returns optional argument table.  The value in the table represent
 * the index into the instruction sequence of the code to set the
 * optional argument.  The last element is the index of the start of the
 * code sequence.
 */
static VALUE iseq_arg_opt_table(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  VALUE ary = rb_ary_new();
  int j;

  for(j = 0; j < iseqdat->arg_opts; ++j)
  {
    rb_ary_push(ary, INT2NUM(iseqdat->arg_opt_table[j]));
  }

  return ary;
}

/* call-seq:
 *   iseq.each(pc_start=0, &block) => nil
 *
 * Yields each instruction in the sequence.
 *
 * If pc_start is supplied, begins iteration at the given offset from
 * the beginning of the sequence.
 *
 */
static VALUE iseq_each(int argc, VALUE * argv, VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  VALUE * seq;
  VALUE pc_start_v = Qnil;
  unsigned long pc_start = 0;

  rb_scan_args(argc, argv, "01", &pc_start_v);

  if(RTEST(pc_start_v))
  {
    pc_start = NUM2ULONG(pc_start_v);
  }

  for(seq = iseqdat->iseq + pc_start;
      seq < iseqdat->iseq + iseqdat->iseq_size;
      )
  {
    VALUE insn = *seq++;
    int op_type_idx;
    int len = insn_len(insn);
    VALUE args = rb_ary_new();

    for(op_type_idx = 0; op_type_idx < len-1; ++op_type_idx, ++seq)
    {
      switch(insn_op_type(insn, op_type_idx))
      {
        case TS_VALUE:
          rb_ary_push(args, *seq);
          break;

        case TS_LINDEX:
        case TS_DINDEX:
        case TS_NUM:
          rb_ary_push(args, INT2FIX(*seq));
          break;

        case TS_ISEQ:
        {
          rb_iseq_t * iseq = (rb_iseq_t *)*seq;
          if(iseq)
          {
            rb_ary_push(args, iseq->self);
          }
          else
          {
            rb_ary_push(args, Qnil);
          }
          break;
        }

        case TS_GENTRY:
        {
          struct global_entry *entry = (struct global_entry *)*seq;
          rb_ary_push(args, ID2SYM(rb_intern(rb_id2name(entry->id))));
          break;
        }

        case TS_OFFSET:
          rb_ary_push(args, LONG2NUM(*seq));
          /* TODO */
          break;

        case TS_VARIABLE:
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_CDHASH:
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_IC:
        {
          NODE * ic = (NODE *)*seq;
          rb_ary_push(args, wrap_node_as(ic, rb_cInlineCache));
          break;
        }

        case TS_ID:
          rb_ary_push(args, ID2SYM(*seq));
          break;
      }
    }

    rb_yield(allocate_instruction(insn, args));
  }

  return Qnil;
}

/* call-seq:
 *   iseq.insn_line(n) => Integer
 *
 * Returns the line number of the nth instruction in the sequence.
 */
static VALUE iseq_insn_line(VALUE self, VALUE n)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  unsigned long pos = NUM2LONG(n);
  unsigned long i, size = iseqdat->insn_info_size;
  struct iseq_insn_info_entry *iiary = iseqdat->insn_info_table;

  for (i = 0; i < size; i++) {
      if (iiary[i].position == pos) {
          return INT2NUM(iiary[i].line_no);
      }
  }
  return Qnil;
}

/* call-seq:
 *   iseq.catch_table => Array of CatchTableEntry
 *
 * Returns the catch table for the sequence
 */
static VALUE iseq_catch_table(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  unsigned long j;

  VALUE catch_table = rb_ary_new();

  for(j = 0; j < iseqdat->catch_table_size; ++j)
  {
    VALUE catch_table_entry = rb_struct_new(
        rb_cCatchTableEntry,
        iseqdat->catch_table[j].type,
        iseqdat->catch_table[j].iseq,
        ULONG2NUM(iseqdat->catch_table[j].start),
        ULONG2NUM(iseqdat->catch_table[j].end),
        ULONG2NUM(iseqdat->catch_table[j].cont),
        ULONG2NUM(iseqdat->catch_table[j].sp));
    rb_ary_push(catch_table, catch_table_entry);
  }

  return catch_table;
}

/* The putobject instruction takes a VALUE as a parameter.  But if this
 * value is a class, we'll end up trying to dump the class!  That's
 * probably not what we want, so we use a placeholder instead.
 */
void convert_modules_to_placeholders(VALUE array)
{
  int j;

  for(j = 0; j < RARRAY_LEN(array); ++j)
  {
    VALUE v = RARRAY_PTR(array)[j];
    if(TYPE(v) == T_ARRAY)
    {
      convert_modules_to_placeholders(v);
    }
    else if(TYPE(v) == T_MODULE || TYPE(v) == T_CLASS)
    {
      VALUE p = rb_class_new_instance(0, 0, rb_cModulePlaceholder);
      VALUE sym = rb_mod_name(v);
      rb_iv_set(p, "name", sym);
      RARRAY_PTR(array)[j] = p;
    }
  }
}

void convert_placeholders_to_modules(VALUE array)
{
  int j;

  for(j = 0; j < RARRAY_LEN(array); ++j)
  {
    VALUE v = RARRAY_PTR(array)[j];
    if(TYPE(v) == T_ARRAY)
    {
      convert_placeholders_to_modules(v);
    }
    else if(CLASS_OF(v) == rb_cModulePlaceholder)
    {
      VALUE sym = rb_ivar_get(v, rb_intern("name"));
      VALUE klass = lookup_module(sym);
      RARRAY_PTR(array)[j] = klass;
    }
  }
}

/*
 * call-seq:
 *   iseq.dump(limit) => String
 *
 * Dumps VM::InstuctionSequence to a string (only available on YARV).
 */
static VALUE iseq_marshal_dump(VALUE self, VALUE limit)
{
  VALUE arr;

  if(rb_safe_level() >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump iseq");
  }

  arr = rb_funcall(self, rb_intern("to_a"), 0);
  convert_modules_to_placeholders(arr);

  return marshal_dump(arr, limit);
}

/*
 * call-seq:
 *   VM::InstructionSequence.load(String) => VM::InstructionSequence
 *
 * Load a VM::InstuctionSequence from a string (only available on YARV).
 */
static VALUE iseq_marshal_load(VALUE klass, VALUE str)
{
  VALUE arr;

  if(   rb_safe_level() >= 4
     || (rb_safe_level() >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load iseq");
  }

  arr = marshal_load(str);
  convert_placeholders_to_modules(arr);

  VALUE iseq = iseq_load(Qnil, arr, 0, Qnil);
  return iseq;
}

void dump_iseq_to_hash(VALUE iseq, VALUE node_hash)
{
  if(RTEST(rb_hash_aref(node_hash, rb_obj_id(iseq))))
  {
    return;
  }

  rb_hash_aset(node_hash, rb_obj_id(iseq), iseq);
}

VALUE load_iseq_from_hash(VALUE iseq, VALUE orig_node_id, VALUE node_hash, VALUE id_hash)
{
  rb_hash_aset(id_hash, orig_node_id, rb_obj_id(iseq));
  return iseq;
}

#endif

void Init_iseq(void)
{
#ifdef RUBY_VM
  VALUE rb_cRubyVM;

  rb_require("internal/node");
  rb_require("internal/module");
  rb_require("internal/vm/instruction");
  rb_require("internal/vm/inline_cache");

  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  rb_cModulePlaceholder = rb_define_class("ModulePlaceholder", rb_cObject);

  if(!rb_const_defined(rb_cObject, rb_intern("RubyVM")))
  {
    rb_define_const(
        rb_cObject,
        "RubyVM",
        rb_const_get(rb_cObject, rb_intern("VM")));
  }

  rb_cRubyVM = rb_define_class("RubyVM", rb_cObject);
  rb_cInlineCache = rb_const_get(rb_cRubyVM, rb_intern("InlineCache"));

  /* For rdoc: rb_cISeq = rb_define_class_under(rb_cRubyVM, "InstructionSequence", rb_cObject) */
  rb_define_method(rb_cISeq, "self", iseq_self, 0);
  rb_define_method(rb_cISeq, "parent_iseq", iseq_parent_iseq, 0);
  rb_define_method(rb_cISeq, "local_iseq", iseq_local_iseq, 0);
  rb_define_method(rb_cISeq, "name", iseq_name, 0);
  rb_define_method(rb_cISeq, "filename", iseq_filename, 0);
  rb_define_method(rb_cISeq, "local_table", iseq_local_table, 0);
  rb_define_method(rb_cISeq, "argc", iseq_argc, 0);
  rb_define_method(rb_cISeq, "arg_simple", iseq_arg_simple, 0);
  rb_define_method(rb_cISeq, "arg_rest", iseq_arg_rest, 0);
  rb_define_method(rb_cISeq, "arg_block", iseq_arg_block, 0);
  rb_define_method(rb_cISeq, "arg_opt_table", iseq_arg_opt_table, 0);
  rb_define_method(rb_cISeq, "each", iseq_each, -1);
  rb_define_method(rb_cISeq, "insn_line", iseq_insn_line, 1);
  rb_define_method(rb_cISeq, "catch_table", iseq_catch_table, 0);
  rb_include_module(rb_cISeq, rb_mEnumerable);
  rb_define_method(rb_cISeq, "_dump", iseq_marshal_dump, 1);
  rb_define_singleton_method(rb_cISeq, "_load", iseq_marshal_load, 1);

  rb_cCatchTableEntry = rb_struct_define(
      0,
      "type",
      "iseq",
      "start",
      "end",
      "cont",
      "sp",
      0);
  rb_const_set(rb_cISeq, rb_intern("CatchTableEntry"), rb_cCatchTableEntry);

  rb_const_set(rb_cISeq, rb_intern("CATCH_TYPE_RESCUE"), CATCH_TYPE_RESCUE);
  rb_const_set(rb_cISeq, rb_intern("CATCH_TYPE_ENSURE"), CATCH_TYPE_ENSURE);
  rb_const_set(rb_cISeq, rb_intern("CATCH_TYPE_RETRY"), CATCH_TYPE_RETRY);
  rb_const_set(rb_cISeq, rb_intern("CATCH_TYPE_BREAK"), CATCH_TYPE_BREAK);
  rb_const_set(rb_cISeq, rb_intern("CATCH_TYPE_REDO"), CATCH_TYPE_REDO);
  rb_const_set(rb_cISeq, rb_intern("CATCH_TYPE_NEXT"), CATCH_TYPE_NEXT);
      
  /* Prevent compiler warnings about unused static functions */
  insn_name(0);
  insn_op_types(0);
#endif
}

