#include "ruby.h"
#include "node.h"
#include "st.h"
#include "nodeinfo.h"
#include "evalinfo.h"
#include "nodewrap.h"
#include "node_type_descrip.h"

static VALUE rb_cNode = Qnil;
static VALUE rb_mMarshal;
static VALUE node_dump_fmt = Qnil;

/* ---------------------------------------------------------------------
 * Node methods
 * ---------------------------------------------------------------------
 */

/*
 * Allocate a new node
 */
static VALUE node_allocate(VALUE klass)
{
  NODE * n = NEW_NIL();
  /* TODO: Need a free function in this case */
  return Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, n);
}

/*
 * Returns a node's flags
 */
static VALUE node_flags(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return INT2NUM(n->flags);
}

/*
 * Returns the file the node is associated with
 */
static VALUE node_nd_file(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return rb_str_new2(n->nd_file);
}

/*
 * Returns an integer representing the type of the node
 */
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

/*
 * Given a Method, returns the Node for that Method.  This can be used
 * to directly copy one class's method to another (using add_method).
 */
static VALUE method_node(VALUE self, VALUE method)
{
  struct METHOD * m;
  Data_Get_Struct(method, struct METHOD, m);
  return Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, m->body);
}

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
 * Node marshalling
 * ---------------------------------------------------------------------
 */

void dump_node_to_hash(NODE * n, VALUE node_hash)
{
  VALUE s1 = Qnil, s2 = Qnil, s3 = Qnil;
  Node_Type_Descrip const *descrip = node_type_descrip(nd_type(n));

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
  /* Check_Type(n, T_NODE); */

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

  /* The id_hash is a temporary, so it is invalidated if an exception is
   * raised.
   */
  rb_hash_aset(id_hash, orig_node_id, node_id(n));

  descrip = node_type_descrip((flags & FL_UMASK) >> FL_USHIFT);
  load_node_elem(descrip->n1, s1, &tmp_node, node_hash, id_hash);
  load_node_elem(descrip->n2, s2, &tmp_node, node_hash, id_hash);
  load_node_elem(descrip->n3, s3, &tmp_node, node_hash, id_hash);

  /* Note that the garbage collector CAN be invoked at this point, so
   * any node object the GC knowns about must be in a consistent state.
   */
  nd_file_buf = ALLOC_N(char, nd_file_len + 1);

  /* 1) We must NOT get an exception from here on out, since we are
   * modifying a live node, and so nd_file_buf won't be leaked.
   * 2) We must NOT invoke the garbage collector from here on out, since
   * we are modifying a live node.
   */
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

/*
 * Dump a node
 */
static VALUE node_dump(VALUE self, VALUE limit)
{
  NODE * n;
  VALUE node_hash = node_to_hash(self);
  VALUE arr = rb_ary_new();
  Data_Get_Struct(self, NODE, n);
  rb_ary_push(arr, node_id(n));
  rb_ary_push(arr, node_hash);
  return marshal_dump(arr, limit);
}

/*
 * Load a dumped node
 */
static VALUE node_load(VALUE klass, VALUE str)
{
  NODE * n = NEW_NIL();
  VALUE arr = marshal_load(str);
  VALUE node_hash = rb_ary_pop(arr);
  VALUE node_id = rb_ary_pop(arr);
  VALUE id_hash = rb_hash_new();
  load_node_from_hash(n, node_id, node_hash, id_hash);
  /* TODO: Need a free function in this case */
  return Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, n);
}

/* ---------------------------------------------------------------------
 * Class marshalling
 * ---------------------------------------------------------------------
 */

static char const * insert_module_sorted_str = 
  "proc { |modules, m|\n"
  "  added = false\n"
  "  a.each_with_index do |other, idx|\n"
  "    if m[0] < other[0] and other[0] < m[0] then\n"
  "      # no relation\n"
  "    elsif other[0] < m[0] then\n"
  "      modules[0..idx] = [m] + modules[0..idx]\n"
  "      added = true\n"
  "      break\n"
  "    end\n"
  "  end\n"
  "  if not added then\n"
  "    modules.push(m)\n"
  "  end\n"
  "}\n";
static VALUE insert_module_sorted_proc = Qnil;

static VALUE generate_method_hash(VALUE module, VALUE method_list)
{
  VALUE methods = rb_hash_new();
  size_t j;
  ID id;
  NODE * body;
  char const * s;
  VALUE v;

  for(j = 0; j < RARRAY(method_list)->len; ++j)
  {
    s = STR2CSTR(RARRAY(method_list)->ptr[j]);
    id = rb_intern(s);
    if(!st_lookup(RCLASS(module)->m_tbl, id, (st_data_t *)&body))
    {
      rb_raise(
          rb_eArgError,
          "module has method %s but I couldn't find it!",
          s);
    }
    v = Data_Wrap_Struct(rb_cNode, rb_gc_mark, 0, body);
    rb_hash_aset(methods, ID2SYM(id), v);
  }

  return methods;
}

static VALUE instance_method_hash(VALUE module)
{
  VALUE instance_method_list =
    rb_class_instance_methods(0, NULL, module);
  return generate_method_hash(module, instance_method_list);
}

static VALUE included_modules_list(VALUE module)
{
  VALUE included_modules = rb_mod_included_modules(module);
  VALUE included_module_list = rb_ary_new();
  size_t j;

  for(j = 0; j < RARRAY(included_modules)->len; ++j)
  {
    rb_ary_push(
        included_module_list,
        rb_mod_name(RARRAY(included_modules)->ptr[j]));
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

    return rb_mod_name(super);
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

static VALUE module_dump(VALUE self, VALUE limit)
{
  VALUE flags = INT2NUM(RBASIC(self)->flags);
  VALUE instance_methods = instance_method_hash(self);
  VALUE class_variables = class_variable_hash(self);
  VALUE included_modules = included_modules_list(self);
  VALUE superclass = superclass_name(self);
  VALUE metaclass;
  VALUE arr = rb_ary_new();

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
  return marshal_dump(arr, INT2NUM(NUM2INT(limit) + 1));
}

static char const * lookup_module_str = 
  "proc { |name|\n"
  "  o = Object\n"
  "  name.split('::').each do |subname|\n"
  "    o = o.const_get(subname)\n"
  "  end\n"
  "  o\n"
  "}\n";
static VALUE lookup_module_proc = Qnil;

static void include_modules(module, included_modules)
{
  size_t j;
  VALUE v;
  VALUE name;

  rb_check_type(included_modules, T_ARRAY);
  for(j = 0; j < RARRAY(included_modules)->len; ++j)
  {
    name = RARRAY(included_modules)->ptr[j];
    v = rb_funcall(lookup_module_proc, rb_intern("call"), 1, name);
    rb_funcall(module, rb_intern("include"), 1, v);
  }
}

static int add_method_iter(VALUE name, VALUE value, VALUE module)
{
  NODE * n;
  rb_check_type(name, T_SYMBOL);
  /* TODO: Check that this is a node */
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

static VALUE module_load(VALUE klass, VALUE str)
{
  VALUE arr = marshal_load(str);
  VALUE metaclass = rb_ary_pop(arr);
  VALUE superclass = rb_ary_pop(arr);
  VALUE included_modules = rb_ary_pop(arr);
  VALUE class_variables = rb_ary_pop(arr);
  VALUE instance_methods = rb_ary_pop(arr);
  VALUE flags = rb_ary_pop(arr);
  VALUE module;

  if(RTEST(superclass))
  {
    rb_check_type(superclass, T_STRING);
    module = rb_class_new(rb_const_get(
            rb_cObject,
            rb_intern(STR2CSTR(superclass))));
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

/* ---------------------------------------------------------------------
 * Initialization
 * ---------------------------------------------------------------------
 */

void Init_nodewrap(void)
{
  rb_cNode = rb_define_class("Node", rb_cObject);
  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  /* TODO: should be a private class method */
  rb_define_singleton_method(rb_cNode, "allocate", node_allocate, 0);

  rb_define_method(rb_cNode, "flags", node_flags, 0);
  rb_define_method(rb_cNode, "nd_file", node_nd_file, 0);
  rb_define_method(rb_cNode, "nd_type", node_nd_type, 0);

  rb_define_method(rb_cNode, "_dump", node_dump, 1);
  rb_define_singleton_method(rb_cNode, "_load", node_load, 1);

  rb_define_singleton_method(rb_cNode, "method_node", method_node, 1);

  // VALUE rb_cModule = rb_define_class("Module", rb_cObject);
  VALUE rb_cModule = rb_const_get(rb_cObject, rb_intern("Module"));
  rb_define_method(rb_cModule, "add_method", add_method, 3);

  rb_define_method(rb_cModule, "_dump", module_dump, 1);
  rb_define_singleton_method(rb_cModule, "_load", module_load, 1);

  node_dump_fmt =
    rb_str_new2("@flags=%d @nd_file=%s @s1=%s @s2=%s @s3=%s");

  insert_module_sorted_proc = rb_eval_string(insert_module_sorted_str);
  lookup_module_proc = rb_eval_string(lookup_module_str);
}
