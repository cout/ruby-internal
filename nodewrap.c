#include "ruby.h"
#include "node.h"
#include "nodeinfo.h"
#include "evalinfo.h"
#include "nodewrap.h"
#include "node_type_descrip.h"

static VALUE rb_cNode = Qnil;
static VALUE rb_mMarshal;
static VALUE node_dump_fmt = Qnil;

static VALUE node_allocate(VALUE klass)
{
  NODE * n = NEW_NIL();
  // TODO: Need a free function in this case
  return Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, n);
}

static VALUE node_flags(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return INT2NUM(n->flags);
}

static VALUE node_nd_file(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return rb_str_new2(n->nd_file);
}

static VALUE node_nd_type(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return INT2NUM(nd_type(n));
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

static VALUE add_method(VALUE klass, VALUE method, VALUE node, VALUE noex)
{
  NODE * n;
  if(!rb_obj_is_kind_of(node, rb_cNode) || rb_obj_class(node) != rb_cNode)
  {
    rb_raise(rb_eTypeError, "Expected Node for 3rd parameter");
  }
  Data_Get_Struct(node, NODE, n);
  rb_add_method(klass, SYM2ID(method), n, NUM2INT(noex));
  return Qnil;
}

static VALUE method_node(VALUE self, VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
  return Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, m->body);
}

VALUE marshal_dump(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("dump"), 1, obj);
}

VALUE marshal_load(VALUE obj)
{
  return rb_funcall(rb_mMarshal, rb_intern("load"), 1, obj);
}

void dump_node_to_hash(NODE * n, VALUE node_hash)
{
  VALUE s1 = Qnil, s2 = Qnil, s3 = Qnil;
  Node_Type_Descrip const *descrip = node_type_descrip(nd_type(n));

  if(RTEST(rb_hash_aref(node_hash, node_id(n))))
  {
    return;
  }

  Check_Type(n, T_NODE);

  s1 = dump_node_elem(descrip->n1, n, node_hash);
  s2 = dump_node_elem(descrip->n2, n, node_hash);
  s3 = dump_node_elem(descrip->n3, n, node_hash);

  VALUE arr = rb_ary_new();
  rb_ary_push(arr, INT2NUM(n->flags));
  rb_ary_push(arr, rb_str_new2(n->nd_file));
  rb_ary_push(arr, s1);
  rb_ary_push(arr, s2);
  rb_ary_push(arr, s3);
  rb_hash_aset(node_hash, node_id(n), arr);
}

void load_node_from_hash(NODE * n, VALUE orig_node_id, VALUE node_hash, VALUE id_hash)
{
  VALUE arr = rb_hash_aref(node_hash, orig_node_id);
  VALUE s3, s2, s1, rb_nd_file, rb_flags;
  unsigned long flags;
  char *nd_file, *nd_file_buf;
  size_t nd_file_len;
  Node_Type_Descrip const *descrip;
  NODE tmp_node;

  nd_set_type(&tmp_node, NODE_NIL);

  if(!RTEST(arr))
  {
    rb_raise(rb_eArgError, "Could not find node %d in hash", NUM2INT(orig_node_id));
  }

  Check_Type(arr, T_ARRAY);
  s3 = rb_ary_pop(arr);
  s2 = rb_ary_pop(arr);
  s1 = rb_ary_pop(arr);
  rb_nd_file = rb_ary_pop(arr);
  rb_flags = rb_ary_pop(arr);

  flags = NUM2INT(rb_flags);
  nd_file = STR2CSTR(rb_nd_file);
  nd_file_len = RSTRING(rb_nd_file)->len; 

  // The id_hash is a temporary, so it is invalidated if an exception is
  // raised.
  rb_hash_aset(id_hash, orig_node_id, node_id(n));

  descrip = node_type_descrip((flags & FL_UMASK) >> FL_USHIFT);
  load_node_elem(descrip->n1, s1, &tmp_node, node_hash, id_hash);
  load_node_elem(descrip->n2, s2, &tmp_node, node_hash, id_hash);
  load_node_elem(descrip->n3, s3, &tmp_node, node_hash, id_hash);

  // Note that the garbage collector CAN be invoked at this point, so
  // any node object the GC knowns about must be in a consistent state.
  nd_file_buf = ALLOC_N(char, nd_file_len + 1);

  // 1) We must NOT get an exception from here on out, since we are
  // modifying a live node, and so nd_file_buf won't be leaked.
  // 2) We must NOT invoke the garbage collector from here on out, since
  // we are modifying a live node.
  memcpy(n, &tmp_node, sizeof(NODE));
  n->flags = flags;
  n->nd_file = nd_file_buf;
  memcpy(n->nd_file, nd_file, nd_file_len);
  n->nd_file[nd_file_len] = '\0';
}

static VALUE node_to_hash(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  VALUE node_hash = rb_hash_new();
  dump_node_to_hash(n, node_hash);
  return node_hash;
}

static VALUE node_from_hash(VALUE klass, VALUE node_id, VALUE node_hash)
{
  NODE * n = NEW_NIL();
  VALUE id_hash = rb_hash_new();
  load_node_from_hash(n, node_id, node_hash, id_hash);
  // TODO: Need a free function in this case
  return Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, n);
}

static VALUE node_dump(VALUE self)
{
  NODE * n;
  VALUE node_hash = node_to_hash(self);
  VALUE arr = rb_ary_new();
  Data_Get_Struct(self, NODE, n);
  rb_ary_push(arr, node_id(n));
  rb_ary_push(arr, node_hash);
  return marshal_dump(arr);
}

static VALUE node_load(VALUE self, VALUE str)
{
  NODE * n;
  VALUE arr = marshal_load(str);
  Data_Get_Struct(self, NODE, n);
  VALUE node_hash = rb_ary_pop(arr);
  VALUE node_id = rb_ary_pop(arr);
  VALUE id_hash = rb_hash_new();
  load_node_from_hash(n, node_id, node_hash, id_hash);
  return Qnil;
}

void Init_nodewrap(void)
{
  rb_cNode = rb_define_class("Node", rb_cObject);
  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  // TODO: should be a private class method
  rb_define_singleton_method(rb_cNode, "allocate", node_allocate, 0);

  rb_define_method(rb_cNode, "flags", node_flags, 0);
  rb_define_method(rb_cNode, "nd_file", node_nd_file, 0);
  rb_define_method(rb_cNode, "nd_type", node_nd_type, 0);

  rb_define_method(rb_cNode, "to_hsh", node_to_hash, 0);
  rb_define_singleton_method(rb_cNode, "from_hsh", node_from_hash, 1);

  rb_define_method(rb_cNode, "_dump_data", node_dump, 0);
  rb_define_method(rb_cNode, "_load_data", node_load, 1);

  rb_define_singleton_method(rb_cNode, "method_node", method_node, 1);
  rb_define_method(rb_cModule, "add_method", add_method, 3);

  node_dump_fmt =
    rb_str_new2("@flags=%d @nd_file=%s @s1=%s @s2=%s @s3=%s");
}

