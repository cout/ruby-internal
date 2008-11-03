#include <ruby.h>
#include "internal/node/ruby_internal_node.h"
#include "method.h"

#ifdef RUBY_VM
#include "vm_core.h"
#endif

#ifndef HAVE_RB_CMETHOD
static VALUE rb_cMethod = Qnil;
#endif

#ifndef HAVE_RB_CUNBOUNDMETHOD
static VALUE rb_cUnboundMethod = Qnil;
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RCLASS(a)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RCLASS(a)->ptr
#endif

static VALUE rb_mMarshal;

static VALUE marshal_dump(VALUE obj, VALUE limit)
{
  return rb_funcall(rb_mMarshal, rb_intern("dump"), 2, obj, limit);
}

static VALUE marshal_load(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("load"), 1, obj);
}

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
  return METHOD_OCLASS(m);
}

/*
 * call-seq:
 *   method.attached_class => Class
 *
 * Given a Method, returns the Class it is attached to.
 */
static VALUE method_attached_class(VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
  return CLASS_OF(m->recv);
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
  if(rb_safe_level() >= 4)
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

  if(rb_safe_level() >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't dump method");
  }

  arr = rb_ary_new();
  Data_Get_Struct(self, struct METHOD, method);
  rb_ary_push(arr, rb_mod_name(METHOD_OCLASS(method)));
  rb_ary_push(arr, rb_mod_name(METHOD_RCLASS(method)));
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

  if(   rb_safe_level() >= 4
     || (rb_safe_level() >= 1 && OBJ_TAINTED(str)))
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
  METHOD_OCLASS(method) =
    rb_funcall(lookup_module_proc, rb_intern("call"), 1, arr[0]);
  METHOD_RCLASS(method) =
    rb_funcall(lookup_module_proc, rb_intern("call"), 1, arr[1]);
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

void Init_method(void)
{
  rb_require("internal/node");

  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  /* For rdoc: rb_cMethod = rb_define_class("Method", rb_cObject) */
#ifndef HAVE_RB_CMETHOD
  rb_cMethod = rb_const_get(rb_cObject, rb_intern("Method"));
#endif
  rb_define_method(rb_cMethod, "receiver", method_receiver, 0);

  /* For rdoc: rb_cUnboundMethod = rb_define_class("UnboundMethod", rb_cObject) */
#ifndef HAVE_RB_CUNBOUNDMETHOD
  rb_cUnboundMethod = rb_const_get(rb_cObject, rb_intern("UnboundMethod"));
#endif
  rb_define_method(rb_cMethod, "origin_class", method_origin_class, 0);
  rb_define_method(rb_cUnboundMethod, "origin_class", method_origin_class, 0);
  rb_define_method(rb_cMethod, "attached_class", method_attached_class, 0);
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

  lookup_module_proc = rb_eval_string(lookup_module_str);
  rb_global_variable(&lookup_module_proc);
}

