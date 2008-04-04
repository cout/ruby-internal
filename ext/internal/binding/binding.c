#include <ruby.h>
#include "internal/node/ruby_internal_node.h"
#include "block.h"

#ifdef RUBY_VM
#include "vm_core.h"
#endif

/*
 * call-seq:
 *   binding.body => Binding
 *
 * Given a Binding, returns the Node for that Binding's body.
 *
 * On YARV, this will returning the Binding's instruction sequence.
 */
static VALUE binding_body(VALUE self)
{
#ifdef RUBY_VM
  rb_binding_t * binding;
  rb_env_t * env;
  GetBindingPtr(self, binding);
  GetEnvPtr(binding->env, env);
  return env->block.iseq->self;
#else
  struct BLOCK * b;
  if(rb_safe_level() >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get binding body");
  }
  Data_Get_Struct(self, struct BLOCK, b);
  return wrap_node(b->body);
#endif
}

void Init_binding(void)
{
  rb_require("internal/node");

  /* For rdoc: rb_cBinding = rb_define_class("Binding", rb_cObject) */
  VALUE rb_cBinding = rb_const_get(rb_cObject, rb_intern("Binding"));
  rb_define_method(rb_cBinding, "body", binding_body, 0);
}

