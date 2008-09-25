#include "internal/node/ruby_internal_node.h"
#include "internal/node/nodeinfo.h"
#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

static VALUE inline_cache_klass(VALUE self)
{
  /* TODO: returning the real value can crash the interpreter */
  return Qnil;
}

static VALUE inline_cache_value(VALUE self)
{
  IC inline_cache = unwrap_node(self);
  return inline_cache->ic_value;
}

static VALUE inline_cache_vmstat(VALUE self)
{
  IC inline_cache = unwrap_node(self);
  return INT2NUM(inline_cache->ic_vmstat);
}

#endif

void Init_inline_cache(void)
{
#ifdef RUBY_VM
  VALUE rb_cNode;
  VALUE rb_cRubyVM;

  rb_require("internal/node/node");

  rb_cNode = rb_define_class("Node", rb_cObject);

  if(!rb_const_defined(rb_cObject, rb_intern("RubyVM")))
  {
    rb_define_const(
        rb_cObject,
        "RubyVM",
        rb_const_get(rb_cObject, rb_intern("VM")));
  }

  rb_cRubyVM = rb_define_class("RubyVM", rb_cObject);
  VALUE rb_cInlineCache = rb_define_class_under(rb_cRubyVM, "InlineCache", rb_cNode);
  rb_define_method(rb_cInlineCache, "klass", inline_cache_klass, 0);
  rb_define_method(rb_cInlineCache, "value", inline_cache_value, 0);
  rb_define_method(rb_cInlineCache, "vmstat", inline_cache_vmstat, 0);
  VALUE inline_cache_members = rb_ary_new();
  rb_ary_push(inline_cache_members, rb_str_new2("klass"));
  rb_ary_push(inline_cache_members, rb_str_new2("value"));
  rb_ary_push(inline_cache_members, rb_str_new2("vmstat"));
  rb_iv_set(rb_cInlineCache, "__member__", inline_cache_members);
  rb_define_singleton_method(rb_cInlineCache, "members", node_s_members, 0);
#endif
}

