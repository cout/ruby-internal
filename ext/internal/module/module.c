#include <ruby.h>
#include "internal/node/ruby_internal_node.h"

#ifdef RUBY_VM
#include <ruby/st.h>
#include "vm_core.h"
#include "eval_intern.h"
#include "cfp.h"
#else
#include <rubysig.h>
#include <st.h>
#endif

#ifndef RCLASS_SUPER
#define RCLASS_SUPER(c) RCLASS(c)->super
#endif

#ifndef RCLASS_IV_TBL
#define RCLASS_IV_TBL(c) RCLASS(c)->iv_tbl
#endif

#ifndef RCLASS_M_TBL
#define RCLASS_M_TBL(c) RCLASS(c)->m_tbl
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

static VALUE rb_cNode;

static VALUE rb_mMarshal;

static VALUE rb_cClass_Restorer;

static VALUE marshal_dump(VALUE obj, VALUE limit)
{
  return rb_funcall(rb_mMarshal, rb_intern("dump"), 2, obj, limit);
}

static VALUE marshal_load(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("load"), 1, obj);
}

#if RUBY_VERSION_CODE >= 180
struct Class_Restorer
{
  VALUE klass;
  struct st_table m_tbl;
  struct st_table iv_tbl;
#ifndef RUBY_VM
  int thread_critical;
#endif
};

#if RUBY_VERSION_CODE >= 180
static void mark_class_restorer(struct Class_Restorer * class_restorer)
{
  rb_mark_tbl(&class_restorer->m_tbl);
  rb_mark_tbl(&class_restorer->iv_tbl);
}

static VALUE create_class_restorer(VALUE klass)
{
  /* On Ruby 1.8, there is a check in marshal_dump() to ensure that
   * the object being dumped has no modifications to its singleton
   * class (e.g. no singleton instance variables, and no singleton
   * methods defined).  Since we need to dump the class's singleton
   * class in order to dump class methods, we need a way around this
   * restriction.  The solution found here temporarily removes the
   * singleton instance variables and singleton methods while the
   * class is being dumped, and sets a special singleton instance
   * variable that restores the tables when dumping is complete.  A
   * hack for sure, but it seems to work.
   */
  struct RClass * singleton_class = RCLASS(CLASS_OF(klass));
  struct Class_Restorer * class_restorer;

  if(!RCLASS_IV_TBL(singleton_class))
  {
    rb_raise(
        rb_eTypeError,
        "can't dump singleton class on Ruby 1.8 without iv_tbl");
  }

  class_restorer = ALLOC(struct Class_Restorer);
  class_restorer->klass = CLASS_OF(klass);
  class_restorer->m_tbl = *RCLASS_M_TBL(singleton_class);
  class_restorer->iv_tbl = *RCLASS_IV_TBL(singleton_class);
#ifndef RUBY_VM
  class_restorer->thread_critical = rb_thread_critical;
#endif
  return Data_Wrap_Struct(
      rb_cClass_Restorer, mark_class_restorer, ruby_xfree,
      class_restorer);
}

static void set_class_restore_state(VALUE klass)
{
  struct RClass * singleton_class = RCLASS(CLASS_OF(klass));
  RCLASS_IV_TBL(singleton_class)->num_entries = 1;
  RCLASS_M_TBL(singleton_class)->num_entries = 0;
#ifndef RUBY_VM
  rb_thread_critical = 1;
#endif
}

static void restore_class(VALUE ruby_class_restorer)
{
  struct Class_Restorer * class_restorer;
  struct RClass * klass;

  Data_Get_Struct(
      ruby_class_restorer,
      struct Class_Restorer,
      class_restorer);
  klass = RCLASS(class_restorer->klass);
  *RCLASS_M_TBL(klass) = class_restorer->m_tbl;
  *RCLASS_IV_TBL(klass) = class_restorer->iv_tbl;
#ifndef RUBY_VM
  rb_thread_critical = class_restorer->thread_critical;
#endif
}

/*
 * call-seq:
 *   class_restorer.dump => String
 *
 * Do not call this function.
 */
static VALUE class_restorer_dump(VALUE ruby_class_restorer, VALUE limit)
{
  restore_class(ruby_class_restorer);
  return rb_str_new2("");
}

/*
 * call-seq:
 *   Nodewrap::ClassRestorer.load => ClassRestorer
 *
 * Do not call this function.
 */
static VALUE class_restorer_load(VALUE klass, VALUE str)
{
  return Qnil;
}

#endif

static VALUE rb_cClass_Restorer = Qnil;

#if RUBY_VERSION_CODE == 180

static VALUE ruby180_marshal_dump(int argc, VALUE * argv, VALUE klass)
{
  VALUE class_restorer = Qnil;

  if(argc >= 1 && (TYPE(argv[0]) == T_CLASS || TYPE(argv[0]) == T_MODULE))
  {
    class_restorer = create_class_restorer(argv[0]);
    set_class_restore_state(argv[0]);
  }

  VALUE str = rb_funcall2(klass, rb_intern("_Nodewrap__orig_dump"), argc, argv);

  if(class_restorer != Qnil)
  {
    restore_class(class_restorer);
  }

  return str;
}

#endif

#if RUBY_VERSION_CODE >= 180
static VALUE module_instance_allocate(VALUE klass)
{
  NEWOBJ(obj, struct RClass);
  OBJSETUP(obj, klass, T_CLASS);
  return (VALUE)obj;
}
#endif

static int add_to_method_hash(ID id, NODE * body, VALUE methods)
{
  VALUE v = wrap_node(body);
  rb_hash_aset(methods, ID2SYM(id), v);

  return ST_CONTINUE;
}

static VALUE instance_method_hash(VALUE module)
{
  VALUE methods = rb_hash_new();
  st_foreach(
      RCLASS(module)->m_tbl,
      add_to_method_hash,
#ifdef ST_DATA_T_DEFINED
      (st_data_t)methods
#else
      methods
#endif
      );
  return methods;
}

static VALUE included_modules_list(VALUE module)
{
  VALUE included_modules = rb_mod_included_modules(module);
  VALUE included_module_list = rb_ary_new();
  size_t j;

  for(j = 0; j < RARRAY_LEN(included_modules); ++j)
  {
    rb_ary_push(
        included_module_list,
        rb_mod_name(RARRAY_PTR(included_modules)[j]));
  }

  return included_module_list;
}

static VALUE superclass_name(VALUE module)
{
  if(TYPE(module) == T_MODULE)
  {
    return Qnil;
  }
  else
  {
    VALUE super = RCLASS_SUPER(module);

    while(TYPE(super) == T_ICLASS)
    {
      super = RCLASS_SUPER(super);
    }

    if(!super)
    {
      return Qnil;
    }

    if(FL_TEST(super, FL_SINGLETON))
    {
      VALUE v = rb_iv_get(super, "__attached__");
      VALUE name = rb_mod_name(v);
      rb_str_cat2(name, "::<Singleton>");
      return name;
    }
    else
    {
      return rb_mod_name(super);
    }
  }
}

static int add_var_to_hash(ID key, VALUE value, VALUE hash)
{
  /* These are special variables and should not be dumped */
  if(   key != rb_intern("__classpath__")
     && key != rb_intern("__classid__")
     && key != rb_intern("__attached__"))
  {
    rb_hash_aset(hash, ID2SYM(key), value);
  }
  return ST_CONTINUE;
}

static VALUE class_variable_hash(VALUE module)
{
  VALUE class_variables = rb_hash_new();
#if RUBY_VERSION_CODE < 190
  struct st_table * iv_tbl = ROBJECT(module)->iv_tbl;
  if (iv_tbl)
  {
    st_foreach(iv_tbl, add_var_to_hash, class_variables);
  }
#else
  rb_ivar_foreach(module, add_var_to_hash, class_variables);
#endif
  return class_variables;
}

static void mark_class_restorer(struct Class_Restorer * class_restorer);
#endif

static char const * lookup_module_str =
  "proc { |name|\n"
  "  o = Object\n"
  "  name.to_s.split('::').each do |subname|\n"
  "    if subname == '<Singleton>' then\n"
  "      o = o.singleton_class\n"
  "    else\n"
  "      o = o.const_get(subname)\n"
  "    end\n"
  "  end\n"
  "  o\n"
  "}\n";
static VALUE lookup_module_proc = Qnil;

VALUE lookup_module(VALUE name)
{
  return rb_funcall(lookup_module_proc, rb_intern("call"), 1, name);
}

static char const * outer_module_str =
  "proc { |name|\n"
  "  o = Object\n"
  "  names = name.to_s.split('::')\n"
  "  names.pop\n"
  "  names.each do |subname|\n"
  "    if subname == '<Singleton>' then\n"
  "      o = o.singleton_class\n"
  "    else\n"
  "      o = o.const_get(subname)\n"
  "    end\n"
  "  end\n"
  "  o\n"
  "}\n";

static VALUE outer_module_proc = Qnil;

static char const * module_name_str =
  "proc { |name|\n"
  "  names = name.to_s.split('::')\n"
  "  names[-1].intern\n"
  "}\n";

static VALUE module_name_proc = Qnil;

#ifdef RUBY_VM

static void set_cref_stack(rb_iseq_t * iseqdat, VALUE klass, VALUE noex)
{
  rb_thread_t * th = GET_THREAD();
#if defined(HAVE_RB_VM_GET_RUBY_LEVEL_NEXT_CFP)
  rb_control_frame_t * cfp = rb_vm_get_ruby_level_next_cfp(th, th->cfp);
#elif defined(HAVE_VM_GET_RUBY_LEVEL_CFP)
  rb_control_frame_t * cfp = vm_get_ruby_level_cfp(th, th->cfp);
#else
#error No function to get cfp
#endif
  iseqdat->cref_stack = NEW_BLOCK(klass);
  iseqdat->cref_stack->nd_visi = noex;
  iseqdat->cref_stack->nd_next = cfp->iseq->cref_stack; /* TODO: use lfp? */
}

/* From iseq.c */
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

#endif

/*
 * call-seq:
 *   class.add_method(id, node or iseq, noex) #=> nil
 *
 * Adds the method as an instance method to the given class.
 *
 * To add a singleton method to a class, add the method to its singleton
 * class.
 */
static VALUE module_add_method(VALUE klass, VALUE id, VALUE node, VALUE noex)
{
  NODE * n = 0;

  if(rb_safe_level() >= 2)
  {
    /* adding a method with the wrong node type can cause a crash */
    rb_raise(rb_eSecurityError, "Insecure: can't add method");
  }

#ifdef RUBY_VM
  if(rb_obj_is_kind_of(node, rb_cISeq))
  {
    rb_iseq_t *iseqdat = iseq_check(node);
    /* TODO: any restrictions on what kinds of iseqs we can add here?
     */
    set_cref_stack(iseqdat, klass, noex);
    iseqdat->klass = klass;
    iseqdat->defined_method_id = SYM2ID(id);
    n = NEW_METHOD(iseqdat->self, klass, NUM2INT(noex));
    goto add_node;
  }
#endif

  if(!rb_obj_is_kind_of(node, rb_cNode))
  {
    rb_raise(
        rb_eTypeError,
        "Expected Node for 2nd parameter, got %s",
        rb_class2name(CLASS_OF(n)));
  }

  Data_Get_Struct(node, NODE, n);

#ifdef RUBY_VM
  if(nd_type(n) != NODE_METHOD)
  {
    rb_raise(
        rb_eTypeError,
        "Expected METHOD node, got %s",
        rb_class2name(CLASS_OF(n)));
  }

  rb_iseq_t *iseqdat = iseq_check((VALUE)n->nd_body);
  set_cref_stack(iseqdat, klass, noex);
  iseqdat->klass = klass;
  iseqdat->defined_method_id = SYM2ID(id);
  n = NEW_METHOD(iseqdat->self, klass, NUM2INT(noex));

add_node:
#endif
  /* TODO: if noex is NOEX_MODFUNC, add this method as a module function
   * (that is, both as an instance and singleton method)
   */
  rb_add_method(klass, SYM2ID(id), n, NUM2INT(noex));
  return Qnil;
}

/* 
 * call-seq:
 *   uninclude(module, ...) => self
 *
 * Removes the specified module(s) from the inheritance chain.
 */
static VALUE module_uninclude(int argc, VALUE * argv, VALUE module)
{
  int i;

  for (i = 0; i < argc; i++)
    Check_Type(argv[i], T_MODULE);
  while (argc--) {
    rb_funcall(argv[argc], rb_intern("remove_features"), 1, module);
    rb_funcall(argv[argc], rb_intern("unincluded"), 1, module);
  }
  return module;
}

/* 
 * call-seq:
 *   remove_features(mod) => mod
 *
 * When this module is unincluded from another, Nodewrap calls
 * remove_features in this module.  The default behavior is to remove
 * the constants, methods, and module variables of this module from
 * _mod_.  If this module has not been included by _mod_, an exception
 * will be raised.
 */
static VALUE module_remove_features(VALUE module, VALUE uninclude)
{
  VALUE prev, mod;

  if(TYPE(uninclude) != T_CLASS && TYPE(uninclude) != T_MODULE)
  {
    Check_Type(uninclude, T_CLASS);
  }

  rb_frozen_class_p(uninclude);
  if(!OBJ_TAINTED(uninclude))
  {
    rb_secure(4);
  }

  OBJ_INFECT(uninclude, module);

  if(RCLASS(uninclude)->m_tbl == RCLASS(module)->m_tbl)
  {
    rb_raise(rb_eArgError, "Cannot remove module from itself");
  }

  prev = uninclude;
  mod = RCLASS_SUPER(uninclude);

  while(mod)
  {
    if(RCLASS(module)->m_tbl == RCLASS(mod)->m_tbl)
    {
      RCLASS_SUPER(prev) = RCLASS_SUPER(mod);
      rb_clear_cache();
      return module;
    }

    if(BUILTIN_TYPE(mod) == T_CLASS)
    {
      break;
    }

    prev = mod;
    mod = RCLASS_SUPER(mod);
  }

  rb_raise(rb_eArgError, "Could not find included module");
  return module;
}

/*
 * call-seq:
 *   module.unincluded(uninclude) => nil
 *
 * Callback when a module is unincluded.  Should not normally be called
 * by the user.
 */
static VALUE module_unincluded(VALUE module, VALUE uninclude)
{
  return Qnil;
}

/*
 * call-seq:
 *   class.real_superclass => Class
 *
 * Return the immediate superclass of a class or module.  This may be a
 * base class, a singleton class, or a module singleton.
 */
VALUE module_real_superclass(VALUE self)
{
  VALUE super = RCLASS_SUPER(self);
  rb_include_module(rb_class_of(super), rb_mKernel);
  return super;
}

/*
 * call-seq:
 *   module.dump(limit) => String
 *
 * Dump a module to a string.  The module will be dumped along with its
 * instance methods, class variables, names of included modules, name of
 * superclass, its entire metaclass, and the name of the class.
 *
 * Note that on ruby 1.8 and newer the module is temporarily modified
 * while dumping in order to allow singleton classes to be dumped.  To
 * prevent access to the modifed module, Thread.critical is temporarily
 * set, then restored to its original value once dumping is complete.
 * Note also that because YARV does not support Thread.critical, the
 * user must synchronize access to the class with a Mutex in order to
 * prevent accessing the modified class.
 */
static VALUE module_dump(VALUE self, VALUE limit)
{
  VALUE flags, instance_methods, class_variables;
  VALUE included_modules, superclass, metaclass, arr, str, class_name;

  limit = INT2NUM(NUM2INT(limit) - 1);

  if(rb_safe_level() >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump module");
  }

  flags = INT2NUM(RBASIC(self)->flags);
  instance_methods = instance_method_hash(self);
  class_variables = class_variable_hash(self);
  included_modules = included_modules_list(self);
  superclass = superclass_name(self);
  arr = rb_ary_new();

  if(FL_TEST(self, FL_SINGLETON))
  {
    metaclass = Qnil;
    class_name = Qnil;
  }
  else
  {
    metaclass = rb_singleton_class(self);
    class_name = rb_class_path(self);
  }

  rb_ary_push(arr, flags);
  rb_ary_push(arr, marshal_dump(instance_methods, limit));
  rb_ary_push(arr, marshal_dump(class_variables, limit));
  rb_ary_push(arr, included_modules);
  rb_ary_push(arr, superclass);
  rb_ary_push(arr, marshal_dump(metaclass, limit));
  rb_ary_push(arr, class_name);

  str = marshal_dump(arr, limit);

#if RUBY_VERSION_CODE > 180
  {
    VALUE class_restorer = create_class_restorer(self);
    rb_iv_set(str, "__class_restorer__", class_restorer);
    set_class_restore_state(self);
  }
#endif

  return str;
}

static void include_modules(VALUE module, VALUE included_modules)
{
  size_t j;
  VALUE v;
  VALUE name;

  rb_check_type(included_modules, T_ARRAY);
  for(j = 0; j < RARRAY_LEN(included_modules); ++j)
  {
    name = RARRAY_PTR(included_modules)[j];
    v = lookup_module(name);
    rb_funcall(module, rb_intern("include"), 1, v);
  }
}

static int add_method_iter(VALUE name, VALUE value, VALUE module)
{
  NODE * n;
  rb_check_type(name, T_SYMBOL);
  if(!rb_obj_is_kind_of(value, rb_cNode))
  {
    rb_raise(
        rb_eTypeError,
        "Expected Node, but got %s",
        rb_class2name(CLASS_OF(value)));
  }
  Data_Get_Struct(value, NODE, n);
  rb_add_method(module, SYM2ID(name), n->nd_body, n->nd_noex);
  return ST_CONTINUE;
}

static void add_methods(VALUE module, VALUE methods)
{
  rb_check_type(methods, T_HASH);
#ifdef RUBY_VM
  if(RHASH(methods)->ntbl)
  {
    st_foreach(RHASH(methods)->ntbl, add_method_iter, module);
  }
#else
  st_foreach(RHASH(methods)->tbl, add_method_iter, module);
#endif
}

static int set_cvar_from_hash(VALUE key, VALUE value, VALUE module)
{
#ifdef RB_CVAR_SET_4ARGS
  rb_cvar_set(module, SYM2ID(key), value, Qtrue);
#else
  rb_cvar_set(module, SYM2ID(key), value);
#endif
  return ST_CONTINUE;
}

static void add_class_variables(VALUE module, VALUE class_variables)
{
  rb_check_type(class_variables, T_HASH);
#ifdef RUBY_VM
  if(RHASH(class_variables)->ntbl)
  {
    st_foreach(RHASH(class_variables)->ntbl, set_cvar_from_hash, module);
  }
#else
  st_foreach(RHASH(class_variables)->tbl, set_cvar_from_hash, module);
#endif
}

/*
 * call-seq:
 *   Module.load(String) => Module
 *
 * Load a module from a string.
 */
static VALUE module_load(VALUE klass, VALUE str)
{
  VALUE arr, class_name, metaclass_str, metaclass, superclass_name,
        included_modules, class_variables_str, class_variables,
        instance_methods_str, instance_methods, flags, module;

  if(   rb_safe_level() >= 4
     || (rb_safe_level() >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load module");
  }

  arr = marshal_load(str);
  class_name = rb_ary_pop(arr);
  metaclass_str = rb_ary_pop(arr);
  superclass_name = rb_ary_pop(arr);
  included_modules = rb_ary_pop(arr);
  class_variables_str = rb_ary_pop(arr);
  instance_methods_str = rb_ary_pop(arr);
  flags = rb_ary_pop(arr);

  if(RTEST(superclass_name))
  {
    VALUE superclass;
    rb_check_type(superclass_name, T_STRING);
    superclass = rb_funcall(
        lookup_module_proc,
        rb_intern("call"),
        1,
        superclass_name);
#if RUBY_VERSION_CODE >= 180
    /* Can't make subclass of Class on 1.8.x */
    module = rb_class_boot(superclass);
    rb_define_alloc_func(module, module_instance_allocate);
#else
    module = rb_class_new(superclass);
#endif
  }
  else
  {
    module = rb_module_new();
  }

  if(!NIL_P(class_name))
  {
    VALUE outer_module = rb_funcall(outer_module_proc, rb_intern("call"), 1, class_name);
    VALUE module_name = rb_funcall(module_name_proc, rb_intern("call"), 1, class_name);
    rb_const_set(outer_module, SYM2ID(module_name), module);
  }

  RBASIC(module)->flags = NUM2INT(flags);
  include_modules(module, included_modules);
  class_variables = marshal_load(class_variables_str);
  add_class_variables(module, class_variables);
  instance_methods = marshal_load(instance_methods_str);
  add_methods(module, instance_methods);

  metaclass = marshal_load(metaclass_str);
  if(RTEST(metaclass))
  {
    rb_singleton_class_attached(metaclass, module);
    RBASIC(module)->klass = metaclass;
  }

  return module;
}

void Init_module(void)
{
  rb_require("internal/node");
  rb_cNode = rb_const_get(rb_cObject, rb_intern("Node"));

  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  /* For rdoc: rb_cModule = rb_define_class("Module", rb_cObject) */
  rb_cModule = rb_const_get(rb_cObject, rb_intern("Module"));
  rb_define_method(rb_cModule, "add_method", module_add_method, 3);
  rb_define_private_method(rb_cModule, "uninclude", module_uninclude, -1);
  rb_define_private_method(rb_cModule, "remove_features", module_remove_features, 1);
  rb_define_private_method(rb_cModule, "unincluded", module_unincluded, 1);
  rb_define_method(rb_cModule, "real_superclass", module_real_superclass, 0);

  lookup_module_proc = rb_eval_string(lookup_module_str);
  rb_global_variable(&lookup_module_proc);

  outer_module_proc = rb_eval_string(outer_module_str);
  rb_global_variable(&outer_module_proc);

  module_name_proc = rb_eval_string(module_name_str);
  rb_global_variable(&module_name_proc);

#if RUBY_VERSION_CODE >= 180
  VALUE rb_mNodewrap = rb_define_module("Nodewrap");
  rb_cClass_Restorer = rb_define_class_under(rb_mNodewrap, "ClassRestorer", rb_cObject);
  rb_define_method(rb_cClass_Restorer, "_dump", class_restorer_dump, 1);
  rb_define_singleton_method(rb_cClass_Restorer, "_load", class_restorer_load, 1);
#endif

#if RUBY_VERSION_CODE == 180
  rb_alias(CLASS_OF(rb_mMarshal), rb_intern("_Nodewrap__orig_dump"), rb_intern("dump"));
  rb_define_singleton_method(rb_mMarshal, "dump", ruby180_marshal_dump, -1);
#endif

  rb_define_method(rb_cModule, "_dump", module_dump, 1);
  rb_define_singleton_method(rb_cModule, "_load", module_load, 1);
}

