#include "nodeinfo.h"
#include "evalinfo.h"
#include "nodewrap.h"
#include "node_type_descrip.h"
#include "builtins.h"
#include "insns_info.h"

#include "ruby.h"
#include "version.h"
#include "rubysig.h"
#include "node.h"
#include "st.h"

#include <ctype.h>

#ifdef RUBY_HAS_YARV
#include "eval_intern.h"
#define ruby_safe_level rb_safe_level()
VALUE iseq_data_to_ary(rb_iseq_t * iseq);
VALUE iseq_load(VALUE self, VALUE data, VALUE parent, VALUE opt);
static VALUE rb_cInstruction = Qnil;
static VALUE rb_cModulePlaceholder = Qnil;
#endif

static VALUE rb_cNode = Qnil;
static VALUE rb_cNodeType = Qnil;
VALUE rb_cNodeSubclass[NODE_LAST];
static VALUE rb_cUnboundProc = Qnil;
static VALUE rb_mMarshal = Qnil;

#if RUBY_VERSION_CODE < 185
static VALUE rb_cMethod = Qnil;
static VALUE rb_cUnboundMethod = Qnil;
#endif

#if RUBY_VERSION_CODE >= 180
struct Class_Restorer
{
  VALUE klass;
  struct st_table m_tbl;
  struct st_table iv_tbl;
  int thread_critical;
};

static VALUE rb_cClass_Restorer = Qnil;

static void mark_class_restorer(struct Class_Restorer * class_restorer);
#endif

#if RUBY_VERSION_CODE < 170
typedef void st_data_t;
#endif

static VALUE wrapped_nodes = Qnil;

/* ---------------------------------------------------------------------
 * Node helper functions
 * ---------------------------------------------------------------------
 */

static void wrapped_nodes_end_proc(VALUE data)
{
  wrapped_nodes = Qnil;
}

static void mark_node(
    void * data)
{
  rb_gc_mark((VALUE)data);
}

static void free_node(
    void * data)
{
  VALUE key, node_id;

  if(wrapped_nodes == Qnil)
  {
    /* We're finalizing at exit, so don't clean up */
    return;
  }

  key = LONG2FIX((long)data / 4);
  node_id = rb_hash_aref(wrapped_nodes, key);

  if(NIL_P(node_id))
  {
    rb_bug("tried to free a node that wasn't wrapped!");
    return;
  }
  rb_funcall(wrapped_nodes, rb_intern("delete"), 1, LONG2NUM((long)data / 4));
}

VALUE wrap_node(NODE * n)
{
  VALUE node_id;

  if(!n)
  {
    return Qnil;
  }

  if(wrapped_nodes == Qnil)
  {
    /* We're finalizing at exit so we can't function properly */
    rb_raise(rb_eRuntimeError, "Unable to wrap node during cleanup");
  }

  node_id = rb_hash_aref(wrapped_nodes, LONG2FIX((long)n / 4));

  if(!NIL_P(node_id))
  {
    return (VALUE)(node_id ^ FIXNUM_FLAG);
  }
  else
  {
    VALUE node = Data_Wrap_Struct(
        rb_cNodeSubclass[nd_type(n)], mark_node, free_node, n);
    VALUE node_id = rb_obj_id(node);
    rb_hash_aset(wrapped_nodes, LONG2FIX((long)n / 4), node_id);
    return node;
  }
}

NODE * unwrap_node(VALUE r)
{
  if(!RTEST(r))
  {
    return 0;
  }
  else
  {
    NODE * n;
    if(TYPE(r) == 0)
    {
      rb_bug("Tried to unwrap recycled node");
    }
    if(!rb_obj_is_kind_of(r, rb_cNode))
    {
      rb_raise(rb_eTypeError, "Expected Node");
    }
    Data_Get_Struct(r, NODE, n);
    return n;
  }
}

/* ---------------------------------------------------------------------
 * Instruction sequence helper functions
 * ---------------------------------------------------------------------
 */

#ifdef RUBY_HAS_YARV
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

/* ---------------------------------------------------------------------
 * Marshalling
 * ---------------------------------------------------------------------
 */

VALUE marshal_dump(VALUE obj, VALUE limit)
{
  return rb_funcall(rb_mMarshal, rb_intern("dump"), 2, obj, limit);
}

VALUE marshal_load(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("load"), 1, obj);
}

/* ---------------------------------------------------------------------
 * Constant lookup
 * ---------------------------------------------------------------------
 */

/* This differs from rb_class2path in that it handles the case of
 * <Singleton>.
 */

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

/* ---------------------------------------------------------------------
 * Node methods
 * ---------------------------------------------------------------------
 */

/*
 * Document-class: Node
 *
 * Node is a wrapper for Ruby's Nodes, which are not objects.  Nodes
 * can be obtained from many of the other methods in the nodewrap
 * library (see Method#body and Proc#body, for example).
 */

#if RUBY_VERSION_CODE >= 180
/*
 * call-seq:
 *   Node.allocate() => Node
 *
 * Allocate a new node.
 */
static VALUE node_allocate(VALUE klass)
{
  NODE * n = NEW_NIL();
  return wrap_node(n);
}
#endif

/*
 * call-seq:
 *   node.address() => Numeric
 *
 * Returns a node's address.
 */
static VALUE node_address(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return ULONG2NUM((unsigned long)(n));
}

/*
 * call-seq:
 *   node.flags() => Numeric
 *
 * Returns a node's flags.
 */
static VALUE node_flags(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return INT2NUM(n->flags);
}

/*
 * call-seq:
 *   node.nd_file => String or nil
 *
 * Returns the file the node is associated with
 */
static VALUE node_nd_file(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  if(n->nd_file)
  {
    return rb_str_new2(n->nd_file);
  }
  else
  {
    return Qnil;
  }
}

/*
 * call-seq:
 *   node.nd_line => Numeric
 *
 * Returns the line number the node is associated with.
 */
static VALUE node_nd_line(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(nd_line(n));
}

/*
 * call-seq:
 *   node.nd_type => NodeType
 *
 * Returns a NodeType structure representing the type of the node.
 */
static VALUE node_nd_type(VALUE self)
{
  NODE * n;
  const Node_Type_Descrip * descrip;
  Data_Get_Struct(self, NODE, n);
  rb_check_type((VALUE)(self), T_DATA);
  descrip = node_type_descrip(nd_type(n));
  return rb_struct_new(
      rb_cNodeType,
      rb_str_new2(descrip->name),
      INT2NUM(descrip->nt));
}

VALUE node_id(NODE * n)
{
  return rb_obj_id((VALUE)n);
}

NODE * id_to_node(VALUE id)
{
  unsigned long n = NUM2INT(id);
  return (NODE *)n;
}

/*
 * call-seq:
 *   node.members => Array of String
 *
 * Return an array of strings containing the names of a node's
 * members.
 */
static VALUE node_members(VALUE node)
{
  return node_s_members(rb_class_of(node));
}

/*
 * call-seq:
 *   node[member] => Object
 *
 * Return the given member of a node.
 */
static VALUE node_bracket(VALUE node, VALUE member)
{
  ID id = SYMBOL_P(member)
    ? SYM2ID(member)
    : rb_intern(STR2CSTR(member));
  return rb_funcall(node, id, 0);
}

/* ---------------------------------------------------------------------
 * NodeType methods
 * ---------------------------------------------------------------------
 */

/*
 * Document-class: NodeType
 *
 * NodeType is an abstraction for the C type of a node.  It is a Struct
 * which has two members, +name+ and +value+.
 */

/*
 * call-seq:
 *   node_type.name => String
 *
 * Returns the name of the node.
 */
static VALUE node_type_to_s(VALUE node_type)
{
  return rb_struct_getmember(node_type, rb_intern("name"));
}

/*
 * call-seq:
 *   node_type.to_i => Numeric
 *
 * Returns an integer representing integer type of a node.  This is the
 * value you would see for the type of the node if you were examining it
 * in gdb.
 */
static VALUE node_type_to_i(VALUE node_type)
{
  return rb_struct_getmember(node_type, rb_intern("value"));
}

/* ---------------------------------------------------------------------
 * Module methods
 * ---------------------------------------------------------------------
 */

#ifdef RUBY_HAS_YARV

static void set_cref_stack(rb_iseq_t * iseqdat, VALUE klass, VALUE noex)
{
  rb_thread_t * th = GET_THREAD();
  rb_control_frame_t * cfp = th_get_ruby_level_cfp(th, th->cfp);
  iseqdat->cref_stack = NEW_BLOCK(klass);
  iseqdat->cref_stack->nd_visi = noex;
  iseqdat->cref_stack->nd_next = cfp->iseq->cref_stack; /* TODO: use lfp? */
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

  if(ruby_safe_level >= 2)
  {
    /* adding a method with the wrong node type can cause a crash */
    rb_raise(rb_eSecurityError, "Insecure: can't add method");
  }

#ifdef RUBY_HAS_YARV
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

#ifdef RUBY_HAS_YARV
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
  mod = RCLASS(uninclude)->super;

  while(mod)
  {
    if(RCLASS(module)->m_tbl == RCLASS(mod)->m_tbl)
    {
      RCLASS(prev)->super = RCLASS(mod)->super;
      rb_clear_cache();
      return module;
    }

    if(BUILTIN_TYPE(mod) == T_CLASS)
    {
      break;
    }

    prev = mod;
    mod = RCLASS(mod)->super;
  }

  rb_raise(rb_eArgError, "Could not find included module");
  return module;
}

static VALUE module_unincluded(VALUE module, VALUE uninclude)
{
  return Qnil;
}

/* ---------------------------------------------------------------------
 * Method methods
 * ---------------------------------------------------------------------
 */

/*
 * call-seq:
 *   method.receiver => Object
 *
 * Given a Method, returns the Object to which it is bound.
 */
static VALUE method_receiver(VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
  return m->recv;
}

/*
 * call-seq:
 *   method.method_id => Symbol
 *
 * Given a Method, returns the Symbol of the method it represents.  If
 * the method is an alias for another method, returns the Symbol of the
 * new method, not the original.  If the method changes name, returns
 * the original name, not the new name.
 */
static VALUE method_id(VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
  return ID2SYM(m->id);
}

/*
 * call-seq:
 *   method.method_oid => Symbol
 *
 * Given a Method, returns the Symbol of the method it represents.  If
 * the method is an alias for another method, returns the Symbol of the
 * original method, not the alias.  If the original method changes name,
 * returns the original name.
 */
static VALUE method_oid(VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
  return ID2SYM(m->oid);
}

/*
 * call-seq:
 *   method.origin_class => Class
 *
 * Given a Method, returns the Class in which the method it represents
 * was defined.  If the method was defined in a base class and
 * Object#method is called on a derived instance of that base class,
 * this method returns the base class.
 */
static VALUE method_origin_class(VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
#if RUBY_VERSION_CODE < 180
  return m->oklass;
#else
  return m->rklass;
#endif
}

/*
 * call-seq:
 *   method.body => Node
 *
 * Given a Method, returns the Node for that Method's body.  This can be
 * used to directly copy one class's method to another (using
 * add_method).
 */
static VALUE method_body(VALUE method)
{
  struct METHOD * m;
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get method body");
  }
  Data_Get_Struct(method, struct METHOD, m);
  return wrap_node(m->body);
}

/*
 * call-seq:
 *   method.dump(limit) => String
 *
 * Dump a Method and the object to which it is bound to a String.  The
 * Method's class will not be dumped, only the name of the class.
 *
 * Unfortunately, this means that methods for anonymous classes can be
 * dumped but cannot be loaded.
 */
static VALUE method_dump(VALUE self, VALUE limit)
{
  struct METHOD * method;
  VALUE arr;

  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump method");
  }

  arr = rb_ary_new();
  Data_Get_Struct(self, struct METHOD, method);
  rb_ary_push(arr, rb_mod_name(method->klass));
#if RUBY_VERSION_CODE >= 180
  rb_ary_push(arr, rb_mod_name(method->rklass));
#else
  rb_ary_push(arr, rb_mod_name(method->oklass));
#endif
  if(rb_class_of(self) == rb_cUnboundMethod)
  {
    rb_ary_push(arr, Qnil);
  }
  else
  {
    rb_ary_push(arr, method->recv);
  }
  rb_ary_push(arr, ID2SYM(method->id));
  rb_ary_push(arr, ID2SYM(method->oid));
  rb_ary_push(arr, method_body(self));

  return marshal_dump(arr, limit);
}

/*
 * call-seq:
 *   Method.load(String) => Method
 *
 * Load a Method from a String.
 */
static VALUE method_load(VALUE klass, VALUE str)
{
  struct METHOD * method;
  VALUE rarr = marshal_load(str);
  VALUE * arr;
  NODE * n;
  VALUE retval;

  if(   ruby_safe_level >= 4
     || (ruby_safe_level >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load method");
  }

  Check_Type(rarr, T_ARRAY);
  if(RARRAY_LEN(rarr) != 6)
  {
    rb_raise(rb_eArgError, "corrupt data");
  }

  /* Create a METHOD object -- doesn't matter which method we use */
  retval = rb_funcall(
      rb_cObject, rb_intern("method"), 1, ID2SYM(rb_intern("__id__")));
  Data_Get_Struct(retval, struct METHOD, method);
  arr = RARRAY_PTR(rarr);
  method->klass =
    rb_funcall(lookup_module_proc, rb_intern("call"), 1, arr[0]);
#if RUBY_VERSION_CODE >= 180
  method->rklass =
    rb_funcall(lookup_module_proc, rb_intern("call"), 1, arr[1]);
#else
  method->oklass =
    rb_funcall(lookup_module_proc, rb_intern("call"), 1, arr[1]);
#endif
  method->recv = arr[2];
  method->id = SYM2ID(arr[3]);
  method->oid = SYM2ID(arr[4]);
  Data_Get_Struct(arr[5], NODE, n);
  method->body = n;

  if(klass == rb_cUnboundMethod)
  {
    retval = rb_funcall(retval, rb_intern("unbind"), 0);
  }

  return retval;
}

/* ---------------------------------------------------------------------
 * Proc methods
 * ---------------------------------------------------------------------
 */

/*
 * call-seq:
 *   proc.body => Node
 *
 * Returns the Proc's body Node.
 */
static VALUE proc_body(VALUE proc)
{
#ifdef RUBY_HAS_YARV
  /* TODO */
  return Qnil;
#else
  struct BLOCK * b;
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get proc body");
  }
  Data_Get_Struct(proc, struct BLOCK, b);
  return wrap_node(b->body);
#endif
}

/*
 * call-seq:
 *   proc.var => Node
 *
 * Returns the Proc's argument Node.
 */
static VALUE proc_var(VALUE proc)
{
#ifdef RUBY_HAS_YARV
  /* TODO */
  return Qnil;
#else
  struct BLOCK * b;
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get proc var");
  }
  Data_Get_Struct(proc, struct BLOCK, b);
  if(b->var == (NODE*)1)
  {
    /* no parameter || */
    return INT2NUM(1);
  }
  else if(b->var == (NODE*)2)
  {
    /* also no params, but I'm not sure how this one gets generated */
    return INT2NUM(2);
  }
  else
  {
    return wrap_node(b->var);
  }
#endif
}

/*
 * call-seq:
 *   proc.dump(limit) => String
 *
 * Dump a Proc to a String.
 */
static VALUE proc_dump(VALUE self, VALUE limit)
{
#ifdef RUBY_HAS_YARV
  /* TODO */
  return Qnil;
#else
  struct BLOCK * b;
  VALUE body, var, arr;

  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump proc");
  }

  Data_Get_Struct(self, struct BLOCK, b);
  body = wrap_node(b->body);
  var = wrap_node(b->var);
  arr = rb_assoc_new(body, var);
  return marshal_dump(arr, limit);
#endif
}

#ifdef RUBY_HAS_YARV

static VALUE create_proc(VALUE klass, VALUE binding, rb_iseq_t * iseq)
{
  VALUE new_proc = rb_funcall(
      rb_cObject, rb_intern("eval"), 2, rb_str_new2("proc { }"), binding);
  rb_proc_t * p;
  GetProcPtr(new_proc, p);
  p->block.iseq = iseq;
  RBASIC(new_proc)->klass = klass;
  return new_proc;
}

#else

static VALUE create_proc(VALUE klass, VALUE binding, NODE * body, NODE * var)
{
  /* Calling eval will do a security check */
  VALUE new_proc = rb_funcall(
      rb_cObject, rb_intern("eval"), 2, rb_str_new2("proc { }"), binding);
  struct BLOCK * b;
  Data_Get_Struct(new_proc, struct BLOCK, b);
  b->body = body;
  b->var = var;
  RBASIC(new_proc)->klass = klass;
  return new_proc;
}

#endif

/*
 * call-seq:
 *   Proc.load(str) => Proc
 *
 * Load a Proc from a String.  When it is loaded, it will be an
 * UnboundProc.
 */
static VALUE proc_load(VALUE klass, VALUE str)
{
#ifdef RUBY_HAS_YARV
  /* TODO */
  return Qnil;
#else
  VALUE arr = marshal_load(str);
  NODE * body, * var;

  if(   ruby_safe_level >= 4
     || (ruby_safe_level >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load proc");
  }

  Check_Type(arr, T_ARRAY);
  body = unwrap_node(RARRAY_PTR(arr)[0]);
  var = unwrap_node(RARRAY_PTR(arr)[1]);
  return create_proc(rb_cUnboundProc, Qnil, body, var);
#endif
}

/*
 * call-seq:
 *   proc.unbind => UnboundProc
 *
 * Create an UnboundProc from a Proc.
 */
static VALUE proc_unbind(VALUE self)
{
#ifdef RUBY_HAS_YARV
  VALUE u;
  rb_proc_t * p, * np;
  GetProcPtr(self, p);
  return create_proc(rb_cUnboundProc, Qnil, p->block.iseq);
#else
  struct BLOCK * b;
  Data_Get_Struct(self, struct BLOCK, b);
  /* no need for a security check to unbind a proc -- though without the
   * ability to bind, this doesn't seem very useful.
   */
  return create_proc(rb_cUnboundProc, Qnil, b->body, b->var);
#endif
}

/*
 * call-seq:
 *   unbound_proc.bind(Binding) => Proc
 *
 * Bind an UnboundProc to a Binding.  Returns a Proc that has been bound
 * to the given binding.
 */
static VALUE unboundproc_bind(VALUE self, VALUE binding)
{
#ifdef RUBY_HAS_YARV
  VALUE u;
  rb_proc_t * p, * np;
  GetProcPtr(self, p);
  return create_proc(rb_cProc, binding, p->block.iseq);
#else
  struct BLOCK * b;
  Data_Get_Struct(self, struct BLOCK, b);
  /* create_proc will do a security check */
  return create_proc(rb_cProc, binding, b->body, b->var);
#endif
}

/*
 * call-seq:
 *   unbound_proc.call => raises TypeError
 *
 * Raises a TypeError; UnboundProc objects cannot be called.
 */
static VALUE unboundproc_call(VALUE self, VALUE args)
{
  rb_raise(rb_eTypeError, "you cannot call unbound proc; bind first");
}

/*
 * call-seq:
 *   unbound_proc.binding => raises TypeError
 *
 * Raises a TypeError; UnboundProc objects have no binding.
 */
static VALUE unboundproc_binding(VALUE self)
{
  rb_raise(rb_eTypeError, "unbound proc has no binding");
}

/* ---------------------------------------------------------------------
 * Binding methods
 * ---------------------------------------------------------------------
 */

/*
 * call-seq:
 *   binding.body => Binding
 *
 * Given a Binding, returns the Node for that Binding.
 */
static VALUE binding_body(VALUE binding)
{
#ifdef RUBY_HAS_YARV
  /* TODO */
  return Qnil;
#else
  struct BLOCK * b;
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get binding body");
  }
  Data_Get_Struct(binding, struct BLOCK, b);
  return wrap_node(b->body);
#endif
}

/* ---------------------------------------------------------------------
 * Node evaluation
 * ---------------------------------------------------------------------
 */

/*
 * call-seq:
 *   node.eval(Object) => Object
 *
 * Evaluate a node with the given object as self and returns the result.
 */
static VALUE node_eval(VALUE node, VALUE self)
{
#ifdef RUBY_HAS_YARV
  /* TODO */
  return Qnil;
#else
  /* Ruby doesn't give us access to rb_eval, so we have to fake it. */
  NODE * n = unwrap_node(node);
  struct BLOCK * b;
  VALUE proc;

  if(ruby_safe_level >= 2)
  {
    /* evaluating a node can cause a crash */
    rb_raise(rb_eSecurityError, "Insecure: can't add method");
  }

  proc = create_proc(rb_cProc, Qnil, n, 0);
  Data_Get_Struct(proc, struct BLOCK, b);
  b->self = self;
  return rb_funcall(proc, rb_intern("call"), 0);
#endif
}

/* ---------------------------------------------------------------------
 * Node marshalling
 * ---------------------------------------------------------------------
 */

void dump_node_to_hash(NODE * n, VALUE node_hash)
{
  VALUE s1 = Qnil, s2 = Qnil, s3 = Qnil;
  Node_Type_Descrip const *descrip = node_type_descrip(nd_type(n));
  VALUE nd_file;
  VALUE arr;

  if(RTEST(rb_hash_aref(node_hash, node_id(n))))
  {
    return;
  }

  if(TYPE(n) != T_NODE)
  {
    rb_raise(
        rb_eTypeError,
        "wrong argument type %s (expected Node)",
        rb_class2name(CLASS_OF(n)));
  }

  s1 = dump_node_elem(descrip->n1, n, node_hash);
  s2 = dump_node_elem(descrip->n2, n, node_hash);
  s3 = dump_node_elem(descrip->n3, n, node_hash);

  nd_file = Qnil;
  if(n->nd_file)
  {
    nd_file = rb_str_new2(n->nd_file);
  }

  arr = rb_ary_new();
  rb_ary_push(arr, INT2NUM(n->flags));
  rb_ary_push(arr, nd_file);
  rb_ary_push(arr, s1);
  rb_ary_push(arr, s2);
  rb_ary_push(arr, s3);

  rb_hash_aset(node_hash, node_id(n), arr);
}

void dump_node_or_iseq_to_hash(VALUE n, VALUE node_hash)
{
#ifdef RUBY_HAS_YARV
  if(TYPE(n) == T_DATA && CLASS_OF(n) == rb_cISeq)
  {
    return dump_iseq_to_hash(n, node_hash);
  }
#endif

  dump_node_to_hash((NODE *)n, node_hash);
}

NODE * load_node_from_hash(VALUE arr, VALUE orig_node_id, VALUE node_hash, VALUE id_hash)
{
  NODE * n = NEW_NIL();
  VALUE s3, s2, s1, rb_nd_file, rb_flags;
  unsigned long flags;
  char *nd_file = 0;
  Node_Type_Descrip const *descrip;
  NODE tmp_node;

  nd_set_type(&tmp_node, NODE_NIL);

  Check_Type(arr, T_ARRAY);
  s3 = rb_ary_pop(arr);
  s2 = rb_ary_pop(arr);
  s1 = rb_ary_pop(arr);
  rb_nd_file = rb_ary_pop(arr);
  rb_flags = rb_ary_pop(arr);
  flags = NUM2INT(rb_flags);
  tmp_node.flags = flags;

  rb_hash_aset(id_hash, orig_node_id, node_id(n));

  descrip = node_type_descrip(nd_type(&tmp_node));
  load_node_elem(descrip->n1, s1, &tmp_node, node_hash, id_hash);
  load_node_elem(descrip->n2, s2, &tmp_node, node_hash, id_hash);
  load_node_elem(descrip->n3, s3, &tmp_node, node_hash, id_hash);

  /* Note that the garbage collector CAN be invoked at this point, so
   * any node object the GC knowns about must be in a consistent state.
   */
  if(rb_nd_file != Qnil)
  {
    Check_Type(rb_nd_file, T_STRING);
    nd_file = rb_source_filename(RSTRING_PTR(rb_nd_file));
  }

  /* 1) We must NOT get an exception from here on out, since we are
   * modifying a live node, and so nd_file_buf won't be leaked.
   * 2) We must NOT invoke the garbage collector from here on out, since
   * we are modifying a live node.
   */
  memcpy(n, &tmp_node, sizeof(NODE));
  n->flags = flags;
  n->nd_file = nd_file;

  return n;
}


VALUE load_node_or_iseq_from_hash(VALUE orig_node_id, VALUE node_hash, VALUE id_hash)
{
  VALUE data = rb_hash_aref(node_hash, orig_node_id);

  if(!RTEST(data))
  {
    rb_raise(rb_eArgError, "Could not find node %d in hash", NUM2INT(orig_node_id));
  }

#ifdef RUBY_HAS_YARV
  if(TYPE(data) == T_DATA)
  {
    return (VALUE)load_iseq_from_hash(data, orig_node_id, node_hash, id_hash);
  }
#endif

  return (VALUE)load_node_from_hash(data, orig_node_id, node_hash, id_hash);
}


static VALUE node_to_hash(NODE * n)
{
  VALUE node_hash;
  node_hash = rb_hash_new();
  dump_node_to_hash(n, node_hash);
  return node_hash;
}

/*
 * call-seq:
 *   Node.compile_string(str) => Node
 *
 * Compile a string into a node.
 */
static VALUE node_compile_string(int argc, VALUE * argv, VALUE self)
{
  NODE * node;
  VALUE str = Qnil, file = Qnil, line = Qnil;

  rb_scan_args(argc, argv, "12", &str, &file, &line);

  file = NIL_P(file) ? rb_str_new2("(compiled)") : file;
  line = NIL_P(line) ? INT2NUM(1) : line;

  node = rb_compile_string(STR2CSTR(file), str, NUM2INT(line));

  if(ruby_nerrs > 0)
  {
#ifdef RUBY_HAS_YARV
    ruby_nerrs = 0;
    rb_exc_raise(GET_THREAD()->errinfo);
#else
    compile_error(0);
#endif
  }

  return wrap_node(node);
}

#ifdef RUBY_HAS_YARV
/*
 * call-seq:
 *   node.bytecode_compile => VM::InstructionSequence
 *
 * Compile a parsed node tree into a bytecode sequence.
 */
static VALUE node_bytecode_compile(int argc, VALUE * argv, VALUE self)
{
  NODE * node = unwrap_node(self);
  VALUE opt = Qnil;
  rb_compile_option_t option;

  rb_scan_args(argc, argv, "01", &opt);
  make_compile_option(&option, opt);

  return rb_iseq_new_with_opt(
      node,
      rb_str_new2("<main>"),
      rb_str_new2(node->nd_file),
      Qfalse,
      ISEQ_TYPE_TOP,
      &option);
}

#endif

/*
 * call-seq:
 *   node._dump => String
 *
 * Dump a node.
 */
static VALUE node_dump(VALUE self, VALUE limit)
{
  NODE * n;
  VALUE node_hash, arr;

  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump node");
  }

  Data_Get_Struct(self, NODE, n);
  node_hash = node_to_hash(n);
  arr = rb_ary_new();
  rb_ary_push(arr, node_id(n));
  rb_ary_push(arr, node_hash);
  VALUE s =  marshal_dump(arr, limit);
  return s;
}

/*
 * call-seq:
 *   Node._load(str) => Node
 *
 * Load a dumped node.
 */
static VALUE node_load(VALUE klass, VALUE str)
{
  VALUE arr, node_hash, node_id, id_hash;
  NODE * n;
  VALUE data;

  if(   ruby_safe_level >= 4
     || (ruby_safe_level >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load node");
  }

  arr = marshal_load(str);
  node_hash = rb_ary_pop(arr);
  node_id = rb_ary_pop(arr);
  id_hash = rb_hash_new();
  data = rb_hash_aref(node_hash, node_id);
  n = load_node_from_hash(data, node_id, node_hash, id_hash);
  /* TODO: Need a free function in this case */
  return wrap_node(n);
}

/* ---------------------------------------------------------------------
 * Class marshalling
 * ---------------------------------------------------------------------
 */

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
    VALUE super = RCLASS(module)->super;

    while(TYPE(super) == T_ICLASS)
    {
      super = RCLASS(super)->super;
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
  if (ROBJECT(module)->iv_tbl) {
    st_foreach(RCLASS(module)->iv_tbl, add_var_to_hash, class_variables);
  }
  return class_variables;
}

/*
 * call-seq:
 *   module.dump(limit) => String
 *
 * Dump a module to a string.
 */
static VALUE module_dump(VALUE self, VALUE limit)
{
  VALUE flags, instance_methods, class_variables;
  VALUE included_modules, superclass, metaclass, arr, str;

  if(ruby_safe_level >= 4)
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
  }
  else
  {
    metaclass = rb_singleton_class(self);
  }

  rb_ary_push(arr, flags);
  rb_ary_push(arr, instance_methods);
  rb_ary_push(arr, class_variables);
  rb_ary_push(arr, included_modules);
  rb_ary_push(arr, superclass);
  rb_ary_push(arr, metaclass);

  str = marshal_dump(arr, INT2NUM(NUM2INT(limit) + 1));

#if RUBY_VERSION_CODE >= 180
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
    struct RClass * singleton_class = RCLASS(CLASS_OF(self));
    struct Class_Restorer * class_restorer;
    VALUE obj;

    if(!singleton_class->iv_tbl)
    {
      rb_raise(
          rb_eTypeError,
          "can't dump singleton class on Ruby 1.8 without iv_tbl");
    }

    class_restorer = ALLOC(struct Class_Restorer);
    class_restorer->klass = self;
    class_restorer->m_tbl = *singleton_class->m_tbl;
    class_restorer->iv_tbl = *singleton_class->iv_tbl;
    class_restorer->thread_critical = rb_thread_critical;
    obj = Data_Wrap_Struct(
        rb_cClass_Restorer, mark_class_restorer, ruby_xfree,
        class_restorer);
    rb_iv_set(self, "__class_restorer__", obj);

    singleton_class->iv_tbl->num_entries = 1;
    singleton_class->m_tbl->num_entries = 0;
    rb_thread_critical = 1;
  }
#endif

  return str;
}

static void include_modules(module, included_modules)
{
  size_t j;
  VALUE v;
  VALUE name;

  rb_check_type(included_modules, T_ARRAY);
  for(j = 0; j < RARRAY_LEN(included_modules); ++j)
  {
    name = RARRAY_PTR(included_modules)[j];
    v = rb_funcall(lookup_module_proc, rb_intern("call"), 1, name);
    rb_funcall(module, rb_intern("include"), 1, v);
  }
}

static int add_method_iter(VALUE name, VALUE value, VALUE module)
{
  NODE * n;
  rb_check_type(name, T_SYMBOL);
  rb_check_type(value, T_NODE);
  Data_Get_Struct(value, NODE, n);
  rb_add_method(module, SYM2ID(name), n->nd_body, n->nd_noex);
  return ST_CONTINUE;
}

static void add_methods(VALUE module, VALUE methods)
{
  rb_check_type(methods, T_HASH);
  st_foreach(RHASH(methods)->tbl, add_method_iter, module);
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
  st_foreach(RHASH(class_variables)->tbl, set_cvar_from_hash, module);
}

/*
 * call-seq:
 *   Module.load(String) => Module
 *
 * Load a module from a string.
 */
static VALUE module_load(VALUE klass, VALUE str)
{
  VALUE arr, metaclass, superclass_name, included_modules;
  VALUE class_variables, instance_methods, flags, module;

  if(   ruby_safe_level >= 4
     || (ruby_safe_level >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load module");
  }

  arr = marshal_load(str);
  metaclass = rb_ary_pop(arr);
  superclass_name = rb_ary_pop(arr);
  included_modules = rb_ary_pop(arr);
  class_variables = rb_ary_pop(arr);
  instance_methods = rb_ary_pop(arr);
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

  RBASIC(module)->flags = NUM2INT(flags);
  include_modules(module, included_modules);
  add_class_variables(module, class_variables);
  add_methods(module, instance_methods);

  if(RTEST(metaclass))
  {
    rb_singleton_class_attached(metaclass, module);
    RBASIC(module)->klass = metaclass;
  }

  return module;
}

#if RUBY_VERSION_CODE >= 180

static VALUE class_restorer_dump(VALUE ruby_class_restorer, VALUE limit)
{
  struct Class_Restorer * class_restorer;
  struct RClass * klass;

  Data_Get_Struct(
      ruby_class_restorer,
      struct Class_Restorer,
      class_restorer);
  klass = RCLASS(class_restorer->klass);
  *klass->m_tbl = class_restorer->m_tbl;
  *klass->iv_tbl = class_restorer->iv_tbl;
  rb_thread_critical = class_restorer->thread_critical;
  return rb_str_new2("");
}

static void mark_class_restorer(struct Class_Restorer * class_restorer)
{
  rb_mark_tbl(&class_restorer->m_tbl);
  rb_mark_tbl(&class_restorer->iv_tbl);
}

#endif

/* ---------------------------------------------------------------------
 * Methods for VM::InstructionSequence
 * ---------------------------------------------------------------------
 */

#ifdef RUBY_HAS_YARV

/* call-seq:
 *   iseq.self => VM::InstructionSequence
 *
 * Returns the self member of the instruction sequence.
 */
static VALUE iseq_self(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  return iseqdat->self;
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
    rb_ary_push(ary, ID2SYM(iseqdat->local_table[j]));
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
 *   iseq.arg_opt_table => true or false
 *
 * Returns optional argument table, which contains instruction sequences
 * for each of the optional arguments.
 */
static VALUE iseq_arg_opt_table(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  VALUE ary = rb_ary_new();
  int j;

  for(j = 0; j < iseqdat->arg_opts; ++j)
  {
    rb_ary_push(ary, iseqdat->arg_opt_tbl[j]);
  }

  return ary;
}

/* call-seq:
 *   iseq.each(&block) => nil
 *
 * Yields each instruction in the sequence.
 */
static VALUE iseq_each(VALUE self)
{
  rb_iseq_t *iseqdat = iseq_check(self);
  VALUE * seq;

  for(seq = iseqdat->iseq; seq < iseqdat->iseq + iseqdat->size; ++seq)
  {
    int insn = *seq;
    int op_type_idx;
    int len = insn_len(insn);
    VALUE args = rb_ary_new();

    for(op_type_idx = 0; op_type_idx < len-1; ++op_type_idx)
    {
      switch(insn_op_type(insn, op_type_idx))
      {
        case TS_VALUE:
          rb_ary_push(args, *++seq);
          break;

        case TS_LINDEX:
        case TS_DINDEX:
        case TS_NUM:
          rb_ary_push(args, INT2FIX(*++seq));
          break;

        case TS_ISEQ:
          ++seq;
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_GENTRY:
          ++seq;
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_OFFSET:
          ++seq;
          rb_ary_push(args, Qnil);
          /* TODO */

        case TS_VARIABLE:
          ++seq;
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_CDHASH:
          ++seq;
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_IC:
          ++seq;
          rb_ary_push(args, Qnil);
          /* TODO */
          break;

        case TS_ID:
          rb_ary_push(args, ID2SYM(*++seq));
          break;
      }
    }

    rb_yield(rb_class_new_instance(
            RARRAY(args)->len,
            RARRAY(args)->ptr,
            instruction_class[insn]));
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
  struct insn_info_struct *iiary = iseqdat->insn_info_tbl;

  for (i = 0; i < size; i++) {
      if (iiary[i].position == pos) {
          return INT2NUM(iiary[i].line_no);
      }
  }
  return Qnil;
}

/* The putobject instruction takes a VALUE as a parameter.  But if this
 * value is a class, we'll end up trying to dump the class!  That's
 * probably not what we want, so we use a placeholder instead.
 */
void convert_modules_to_placeholders(VALUE array)
{
  int j;

  for(j = 0; j < RARRAY(array)->len; ++j)
  {
    VALUE v = RARRAY(array)->ptr[j];
    if(TYPE(v) == T_ARRAY)
    {
      convert_modules_to_placeholders(v);
    }
    else if(TYPE(v) == T_MODULE || TYPE(v) == T_CLASS)
    {
      VALUE p = rb_class_new_instance(0, 0, rb_cModulePlaceholder);
      VALUE sym = rb_mod_name(p);
      rb_ivar_set(p, rb_intern("name"), sym);
      RARRAY(array)->ptr[j] = p;
    }
  }
}

void convert_placeholders_to_modules(VALUE array)
{
  int j;

  for(j = 0; j < RARRAY(array)->len; ++j)
  {
    VALUE v = RARRAY(array)->ptr[j];
    if(TYPE(v) == T_ARRAY)
    {
      convert_placeholders_to_modules(v);
    }
    else if(CLASS_OF(v) == rb_cModulePlaceholder)
    {
      VALUE sym = rb_ivar_get(v, rb_intern("name"));
      VALUE klass = 
        rb_funcall(lookup_module_proc, rb_intern("call"), 1, sym);
      RARRAY(array)->ptr[j] = klass;
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

  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump iseq");
  }

  arr = iseq_data_to_ary((rb_iseq_t *)DATA_PTR(self));
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

  if(   ruby_safe_level >= 4
     || (ruby_safe_level >= 1 && OBJ_TAINTED(str)))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't load iseq");
  }

  arr = marshal_load(str);
  convert_placeholders_to_modules(arr);

  return iseq_load(Qnil, arr, 0, Qnil);
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

/* ---------------------------------------------------------------------
 * Methods for VM::Instruction
 * ---------------------------------------------------------------------
 */

#ifdef RUBY_HAS_YARV


static VALUE instruction_initialize(int argc, VALUE * argv, VALUE self)
{
  VALUE operands = rb_ary_new4(argc, argv);
  rb_ivar_set(self, rb_intern("@operands"), operands);
  return Qnil;
}

static VALUE instruction_operands(VALUE self)
{
  return rb_ivar_get(self, rb_intern("@operands"));
}

#endif

/* ---------------------------------------------------------------------
 * Eval tree
 * ---------------------------------------------------------------------
 */

/* TODO Not quite sure how to get BEGIN nodes on 1.9.x... */
#if RUBY_VERSION_CODE < 190
extern NODE *ruby_eval_tree_begin;
#endif
extern NODE *ruby_eval_tree;

#if RUBY_VERSION_CODE < 190
static VALUE ruby_eval_tree_begin_getter()
{
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get eval tree");
  }

  if(ruby_eval_tree_begin)
  {
    return wrap_node(ruby_eval_tree_begin);
  }
  else
  {
    return Qnil;
  }
}

static void ruby_eval_tree_begin_setter()
{
  rb_raise(rb_eNotImpError, "ruby_eval_tree_begin_setter() not implemented");
}
#endif

static VALUE ruby_eval_tree_getter()
{
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get eval tree");
  }

  if(ruby_eval_tree)
  {
    return wrap_node(ruby_eval_tree);
  }
  else
  {
    return Qnil;
  }
}

static void ruby_eval_tree_setter()
{
  rb_raise(rb_eNotImpError, "ruby_eval_tree_setter() not implemented");
}

/* ---------------------------------------------------------------------
 * Methods for printing class tree
 * ---------------------------------------------------------------------
 */

/*
 * call-seq:
 *   class.real_superclass => Class
 *
 * Return the immediate superclass of a class or module.  This may be a
 * base class, a singleton class, or a module singleton.
 */
VALUE real_superclass(VALUE self)
{
  rb_include_module(rb_class_of(RCLASS(self)->super), rb_mKernel);
  return RCLASS(self)->super;
}

/*
 * call-seq:
 *   class.real_class => Class
 *
 * Return the object's first immediate ancestor; this may be the
 * object's class, its singleton class, or a module singleton.
 */
VALUE real_class(VALUE self)
{
  return RBASIC(self)->klass;
}

/*
 * call-seq:
 *   class.singleton? => true or false
 *
 * Return true if this object is a singleton (that is, it has the
 * FL_SINGLETON flag set).
 */
VALUE is_singleton(VALUE self)
{
  return FL_TEST(self, FL_SINGLETON) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   class.has_singleton? => true or false
 *
 * Return true if this object has a singleton class.
 */
VALUE has_singleton(VALUE self)
{
  return FL_TEST(RBASIC(self)->klass, FL_SINGLETON) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   class.singleton_class => Class
 *
 * Return the object's singleton class.  Creats a new singleton class
 * for the object if it does not have one.  See RCR#231.
 */
VALUE singleton_class(VALUE self)
{
  return rb_singleton_class(self);
}

/* ---------------------------------------------------------------------
 * Initialization
 * ---------------------------------------------------------------------
 */

void Init_nodewrap(void)
{
  VALUE rb_cBinding, rb_cModule, rb_mNoex;

  {
    int actual_ruby_version_code = 0;
    VALUE ruby_version_str = rb_const_get(rb_cObject, rb_intern("RUBY_VERSION"));
    char const * s = STR2CSTR(ruby_version_str);

    for(; *s != '\0'; ++s)
    {
      if(isdigit(*s))
      {
        actual_ruby_version_code *= 10;
        actual_ruby_version_code += *s - '0';
      }
    }

    if(actual_ruby_version_code != RUBY_VERSION_CODE)
    {
      rb_raise(
          rb_eRuntimeError,
          "This version of nodewrap was built with a different version of ruby "
          "(built with %d, running on %d)",
          RUBY_VERSION_CODE,
          actual_ruby_version_code);
    }
  }

  rb_cNode = rb_define_class("Node", rb_cObject);

#if RUBY_VERSION_CODE >= 180
  rb_define_alloc_func(rb_cNode, node_allocate);
#endif

  rb_define_method(rb_cNode, "address", node_address, 0);
  rb_define_method(rb_cNode, "flags", node_flags, 0);
  rb_define_method(rb_cNode, "nd_file", node_nd_file, 0);
  rb_define_method(rb_cNode, "nd_line", node_nd_line, 0);
  rb_define_method(rb_cNode, "nd_type", node_nd_type, 0);
  rb_define_method(rb_cNode, "members", node_members, 0);
  rb_define_method(rb_cNode, "eval", node_eval, 1);
  rb_define_method(rb_cNode, "[]", node_bracket, 1);
  rb_define_singleton_method(rb_cNode, "compile_string", node_compile_string, -1);
#ifdef RUBY_HAS_YARV
  rb_define_method(rb_cNode, "bytecode_compile", node_bytecode_compile, -1);
#endif
  rb_define_method(rb_cNode, "_dump", node_dump, 1);
  rb_define_singleton_method(rb_cNode, "_load", node_load, 1);

  define_node_subclass_methods();

  /* For rdoc: rb_cNodeType = rb_define_class("NodeType", rb_cObject) */
  rb_cNodeType = rb_funcall(
      rb_cStruct,
      rb_intern("new"),
      2,
      ID2SYM(rb_intern("name")),
      ID2SYM(rb_intern("value")));
  rb_const_set(rb_cNode, rb_intern("Type"), rb_cNodeType);
  rb_define_method(rb_cNodeType, "to_s", node_type_to_s, 0);
  rb_define_method(rb_cNodeType, "to_i", node_type_to_i, 0);

  /* For rdoc: rb_cProc = rb_define_class("Proc", rb_cObject) */
  rb_define_method(rb_cProc, "body", proc_body, 0);
  rb_define_method(rb_cProc, "var", proc_var, 0);
  rb_define_method(rb_cProc, "unbind", proc_unbind, 0);
  rb_define_method(rb_cProc, "_dump", proc_dump, 1);
  rb_define_singleton_method(rb_cProc, "_load", proc_load, 1);

  rb_cUnboundProc = rb_define_class("UnboundProc", rb_cProc);
  rb_define_method(rb_cUnboundProc, "bind", unboundproc_bind, 1);
  rb_define_method(rb_cUnboundProc, "call", unboundproc_call, -2);
  rb_define_method(rb_cUnboundProc, "[]", unboundproc_call, -2);
  rb_define_method(rb_cUnboundProc, "binding", unboundproc_binding, 0);

  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  /* For rdoc: rb_cMethod = rb_define_class("Method", rb_cObject) */
#if RUBY_VERSION_CODE < 185
  rb_cMethod = rb_const_get(rb_cObject, rb_intern("Method"));
#endif
  rb_define_method(rb_cMethod, "receiver", method_receiver, 0);
  rb_define_method(rb_cMethod, "origin_class", method_origin_class, 0);

  /* For rdoc: rb_cUnboundMethod = rb_define_class("UnboundMethod", rb_cObject) */
#if RUBY_VERSION_CODE < 185
  rb_cUnboundMethod = rb_const_get(rb_cObject, rb_intern("UnboundMethod"));
#endif
  rb_define_method(rb_cMethod, "method_id", method_id, 0);
  rb_define_method(rb_cUnboundMethod, "method_id", method_id, 0);
  rb_define_method(rb_cMethod, "method_oid", method_oid, 0);
  rb_define_method(rb_cUnboundMethod, "method_oid", method_oid, 0);
  rb_define_method(rb_cMethod, "body", method_body, 0);
  rb_define_method(rb_cUnboundMethod, "body", method_body, 0);
  rb_define_method(rb_cMethod, "_dump", method_dump, 1);
  rb_define_method(rb_cUnboundMethod, "_dump", method_dump, 1);
  rb_define_singleton_method(rb_cMethod, "_load", method_load, 1);
  rb_define_singleton_method(rb_cUnboundMethod, "_load", method_load, 1);

  /* For rdoc: rb_cBinding = rb_define_class("Binding", rb_cObject) */
  rb_cBinding = rb_const_get(rb_cObject, rb_intern("Binding"));
  rb_define_method(rb_cBinding, "body", binding_body, 0);

  /* For rdoc: rb_cModule = rb_define_class("Module", rb_cObject) */
  rb_cModule = rb_const_get(rb_cObject, rb_intern("Module"));
  rb_define_method(rb_cModule, "add_method", module_add_method, 3);
  rb_define_private_method(rb_cModule, "uninclude", module_uninclude, -1);
  rb_define_private_method(rb_cModule, "remove_features", module_remove_features, 1);
  rb_define_private_method(rb_cModule, "unincluded", module_unincluded, 1);

  lookup_module_proc = rb_eval_string(lookup_module_str);
  rb_global_variable(&lookup_module_proc);

#if RUBY_VERSION_CODE >= 180
  rb_cClass_Restorer = rb_class_new_instance(0, 0, rb_cClass);
  rb_define_method(rb_cClass_Restorer, "_dump", class_restorer_dump, 1);
  rb_global_variable(&rb_cClass_Restorer);
  rb_define_method(rb_cModule, "_dump", module_dump, 1);
  rb_define_singleton_method(rb_cModule, "_load", module_load, 1);
#else
  rb_define_method(rb_cModule, "_dump", module_dump, 1);
  rb_define_singleton_method(rb_cModule, "_load", module_load, 1);
#endif

  wrapped_nodes = rb_hash_new();

  rb_global_variable(&wrapped_nodes);
  rb_set_end_proc(wrapped_nodes_end_proc, Qnil);

  /*
   * Document-module: Noex
   *
   * The Noex module contains all the Noex constants from node.h.  These
   * constants can be passed to Module#add_method as modifiers to the
   * method being added.
   * 
   * [+PUBLIC+]     Method is public.
   * [+UNDEF+]      Method is undefined.
   * [+PRIVATE+]    Method is private.
   * [+PROTECTED+]  Method is protected.
   *
   * The following are available in Ruby 1.8 and below:
   * [+CFUNC+]      Method is a C function.
   *
   * And these are available in Ruby 1.9 and above:
   * [+NOSUPER+]    ???
   * [+MASK+]       ???
   */
  rb_mNoex = rb_define_module("Noex");
  rb_define_const(rb_mNoex, "PUBLIC",    INT2NUM(NOEX_PUBLIC));
  rb_define_const(rb_mNoex, "UNDEF",     INT2NUM(NOEX_UNDEF));
  rb_define_const(rb_mNoex, "PRIVATE",   INT2NUM(NOEX_PRIVATE));
  rb_define_const(rb_mNoex, "PROTECTED", INT2NUM(NOEX_PROTECTED));
#if RUBY_VERSION_CODE < 170
  rb_define_const(rb_mNoex, "CFUNC",     INT2NUM(NOEX_CFUNC));
#else
  rb_define_const(rb_mNoex, "NOSUPER",   INT2NUM(NOEX_NOSUPER));
  rb_define_const(rb_mNoex, "MASK",      INT2NUM(NOEX_MASK));
#endif

#if RUBY_VERSION_CODE < 190
  rb_define_virtual_variable(
      "$ruby_eval_tree_begin",
      ruby_eval_tree_begin_getter,
      ruby_eval_tree_begin_setter);
#endif

  rb_define_virtual_variable(
      "$ruby_eval_tree",
      ruby_eval_tree_getter,
      ruby_eval_tree_setter);

  rb_define_method(rb_cModule, "real_superclass", real_superclass, 0);
  rb_define_method(rb_mKernel, "real_class", real_class, 0);
  rb_define_method(rb_mKernel, "singleton?", is_singleton, 0);
  rb_define_method(rb_mKernel, "has_singleton?", has_singleton, 0);
  rb_define_method(rb_mKernel, "singleton_class", singleton_class, 0);

#ifdef RUBY_HAS_YARV
  /* For rdoc: rb_cVM = rb_define_class("VM", rb_cObject); */
  /* For rdoc: rb_cISeq = rb_define_class_under(rb_cVM, "InstructionSequence", rb_cObject) */
  rb_define_method(rb_cISeq, "self", iseq_self, 0);
  rb_define_method(rb_cISeq, "name", iseq_name, 0);
  rb_define_method(rb_cISeq, "filename", iseq_filename, 0);
  rb_define_method(rb_cISeq, "local_table", iseq_local_table, 0);
  rb_define_method(rb_cISeq, "argc", iseq_argc, 0);
  rb_define_method(rb_cISeq, "arg_simple", iseq_arg_simple, 0);
  rb_define_method(rb_cISeq, "arg_rest", iseq_arg_rest, 0);
  rb_define_method(rb_cISeq, "arg_block", iseq_arg_block, 0);
  rb_define_method(rb_cISeq, "arg_opt_table", iseq_arg_opt_table, 0);
  rb_define_method(rb_cISeq, "each", iseq_each, 0);
  rb_define_method(rb_cISeq, "insn_line", iseq_insn_line, 1);
  rb_include_module(rb_cISeq, rb_mEnumerable);
  rb_define_method(rb_cISeq, "_dump", iseq_marshal_dump, 1);
  rb_define_singleton_method(rb_cISeq, "_load", iseq_marshal_load, 1);

  rb_cInstruction = rb_define_class_under(rb_cVM, "Instruction", rb_cObject);
  rb_define_method(rb_cInstruction, "initialize", instruction_initialize, -1);
  rb_define_method(rb_cInstruction, "operands", instruction_operands, 0);
  rb_undef_method(rb_cInstruction, "new");

  define_instruction_subclasses(rb_cInstruction);

  rb_cModulePlaceholder = rb_define_class("ModulePlaceholder", rb_cObject);
#endif
}

