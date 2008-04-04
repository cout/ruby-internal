#include <ruby.h>
#include "internal/node/ruby_internal_node.h"
#include "block.h"

static VALUE rb_cUnboundProc = Qnil;

static VALUE rb_mMarshal;

static VALUE marshal_dump(VALUE obj, VALUE limit)
{
  return rb_funcall(rb_mMarshal, rb_intern("dump"), 2, obj, limit);
}

static VALUE marshal_load(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("load"), 1, obj);
}

/*
 * call-seq:
 *   proc.body => Node
 *
 * Returns the Proc's body Node.
 *
 * On YARV, this will return the instruction sequence for the proc's
 * block.
 */
static VALUE proc_body(VALUE self)
{
#ifdef RUBY_VM
  rb_proc_t * p;
  GetProcPtr(self, p);
  return p->block.iseq->self;
#else
  struct BLOCK * b;
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get proc body");
  }
  Data_Get_Struct(self, struct BLOCK, b);
  return wrap_node(b->body);
#endif
}

#ifndef RUBY_VM
/*
 * call-seq:
 *   proc.var => Node
 *
 * Returns the Proc's argument Node.
 *
 * This method is undefined on YARV.
 */
static VALUE proc_var(VALUE self)
{
  struct BLOCK * b;
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get proc var");
  }
  Data_Get_Struct(self, struct BLOCK, b);
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
}
#endif

/*
 * call-seq:
 *   proc.dump(limit) => String
 *
 * Dump a Proc to a String.
 */
static VALUE proc_dump(VALUE self, VALUE limit)
{
  if(ruby_safe_level >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump proc");
  }

  {
#ifdef RUBY_VM
    rb_proc_t * p;
    VALUE iseq;
    rb_iseq_t * iseqdat;
    GetProcPtr(self, p);
    iseq = p->block.iseq->self;
    iseqdat = iseq_check(iseq);
    iseqdat->type = ISEQ_TYPE_TOP; /* TODO: is this right? */
    VALUE str = marshal_dump(iseq, limit);
    return str;
#else
    struct BLOCK * b;
    VALUE body, var, arr;

    Data_Get_Struct(self, struct BLOCK, b);
    body = wrap_node(b->body);
    var = wrap_node(b->var);
    arr = rb_assoc_new(body, var);
    return marshal_dump(arr, limit);
#endif
  }
}

#ifdef RUBY_VM

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
 *   Proc.load(str) => UnboundProc
 *
 * Load a Proc from a String.  When it is loaded, it will be an
 * UnboundProc, until it is bound to a Binding with UnboundProc#bind.
 */
static VALUE proc_load(VALUE klass, VALUE str)
{
#ifdef RUBY_VM
  VALUE iseq = marshal_load(str);
  return create_proc(rb_cUnboundProc, Qnil, iseq_check(iseq));
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
#ifdef RUBY_VM
  rb_proc_t * p;
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
 *   proc.push(anotherProc) => self
 *
 * Append the body of anotherProc onto proc.
 */
static VALUE proc_push(VALUE self, VALUE other)
{
#ifdef RUBY_VM
  rb_raise(rb_eRuntimeError, "Proc#push not implemented yet for YARV");
#else
  struct BLOCK * b1;
  struct BLOCK * b2;
  Data_Get_Struct(self, struct BLOCK, b1);
  Data_Get_Struct(other, struct BLOCK, b2);
  b1->body = NEW_NODE(NODE_BLOCK, b1->body, 0, b2->body);
  return self;
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
#ifdef RUBY_VM
  rb_proc_t * p;
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

void Init_proc(void)
{
  rb_require("internal/node");

  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  /* For rdoc: rb_cProc = rb_define_class("Proc", rb_cObject) */
  rb_define_method(rb_cProc, "body", proc_body, 0);
#ifndef RUBY_VM
  rb_define_method(rb_cProc, "var", proc_var, 0);
#endif
  rb_define_method(rb_cProc, "unbind", proc_unbind, 0);
  rb_define_method(rb_cProc, "<<", proc_push, 1);
  rb_define_method(rb_cProc, "push", proc_push, 1);
  rb_define_method(rb_cProc, "_dump", proc_dump, 1);
  rb_define_singleton_method(rb_cProc, "_load", proc_load, 1);

  rb_cUnboundProc = rb_define_class("UnboundProc", rb_cProc);
  rb_define_method(rb_cUnboundProc, "bind", unboundproc_bind, 1);
  rb_define_method(rb_cUnboundProc, "call", unboundproc_call, -2);
  rb_define_method(rb_cUnboundProc, "[]", unboundproc_call, -2);
  rb_define_method(rb_cUnboundProc, "binding", unboundproc_binding, 0);
}

