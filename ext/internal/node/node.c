#include "nodeinfo.h"
#include "ruby_internal_node.h"
#include "node_type_descrip.h"
#include "block.h"
#include "builtins.h"
#include "internal/vm/iseq/internal_iseq.h"

#include <ruby.h>

#ifdef RUBY_VM
#include <ruby/signal.h>
#include <ruby/st.h>
#else
#include <rubysig.h>
#include <st.h>
#endif

#include <ctype.h>

#ifdef RUBY_VM
#include "eval_intern.h"
#endif

#ifdef HAVE_ISEQ_H
#include "iseq.h"
#endif

static VALUE rb_cNode = Qnil;
static VALUE rb_cNodeType = Qnil;
VALUE rb_cNodeSubclass[NODE_LAST];
static VALUE rb_mMarshal = Qnil;

#ifndef HAVE_TYPE_ST_DATA_T
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

VALUE wrap_node_as(NODE * n, VALUE klass)
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
    VALUE node = Data_Wrap_Struct(klass, mark_node, free_node, n);
    VALUE node_id = rb_obj_id(node);
    rb_hash_aset(wrapped_nodes, LONG2FIX((long)n / 4), node_id);
    return node;
  }
}

VALUE wrap_node(NODE * n)
{
  if(!n)
  {
    return Qnil;
  }

  if(nd_type(n) > NODE_LAST || rb_cNodeSubclass[nd_type(n)] == Qnil)
  {
    rb_raise(rb_eRuntimeError, "Unknown node type %d", nd_type(n));
  }

  return wrap_node_as(n, rb_cNodeSubclass[nd_type(n)]);
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

#ifdef HAVE_RB_DEFINE_ALLOC_FUNC
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
#ifdef HAVE_RB_SOURCE_FILENAME
  if(n->nd_file)
  {
    return rb_str_new2(n->nd_file);
  }
  else
  {
    return Qnil;
  }
#else
  /* nd_file has been removed in 1.9 */
  return Qnil;
#endif
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

#ifdef HAVE_RB_PROTECT_INSPECT
static VALUE node_inspect_protect(VALUE node)
#else
static VALUE node_inspect_protect(VALUE node, VALUE dummy, int recur)
#endif
{
  VALUE str = rb_str_new2("#<");
  rb_str_cat2(str, rb_class2name(CLASS_OF(node)));
  rb_str_cat2(str, " ");
  VALUE members = node_members(node);
  int j;


  for(j = 0; j < RARRAY(members)->len; ++j)
  {
    VALUE name = RARRAY(members)->ptr[j];
    VALUE value = node_bracket(node, name);
    rb_str_append(str, name);
    rb_str_cat2(str, "=");
    if(TYPE(value) == T_NODE)
    {
      rb_str_append(str, rb_funcall(value, rb_intern("to_s"), 0));
    }
    else
    {
      rb_str_append(str, rb_funcall(value, rb_intern("inspect"), 0));
    }
    if(j != RARRAY(members)->len - 1)
    {
      rb_str_cat2(str, ", ");
    }
  }

  rb_str_cat2(str, ">");

  return str;
}

/*
 * call-seq:
 *   node.inspect => String
 *
 * Returns a string representation of the node's data.
 */
static VALUE node_inspect(VALUE node)
{
#ifdef HAVE_RB_PROTECT_INSPECT
  if(rb_inspecting_p(node))
  {
    VALUE str = rb_str_new2("#<");
    rb_str_cat2(str, rb_class2name(CLASS_OF(node)));
    rb_str_cat2(str, ":...>");
    return str;
  }
  else
  {
    return rb_protect_inspect(node_inspect_protect, node, 0);
  }
#else
  return rb_exec_recursive(node_inspect_protect, node, 0);
#endif
}

/*
 * call-seq:
 *   node.nd_type => NodeType
 *
 * Returns a NodeType structure representing the type of the node.
 */
static VALUE node_s_type(VALUE self)
{
  const Node_Type_Descrip * descrip;
  descrip = node_type_descrip(
      NUM2INT(rb_iv_get(self, "__type__")));
  return rb_struct_new(
      rb_cNodeType,
      rb_str_new2(descrip->name),
      INT2NUM(descrip->nt));
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
 * Node evaluation
 * ---------------------------------------------------------------------
 */

/* TODO: copied from proc.c */

#ifndef RUBY_VM

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

#else

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

static VALUE create_proc(VALUE klass, VALUE binding, rb_iseq_t * iseq)
{ 
  VALUE new_proc;
  rb_proc_t * p;

  if(binding == Qnil)
  {
    binding = rb_binding_new();
  }
  new_proc = rb_funcall(
      rb_cObject, rb_intern("eval"), 2, rb_str_new2("proc { }"), binding);
  GetProcPtr(new_proc, p);
  p->block.iseq = iseq;
  RBASIC(new_proc)->klass = klass;
  return new_proc;
}

#endif

/* TODO: It would be nicer if we could eval the node in the current
 * scope, but we have to create a new scope both on 1.8 and on 1.9. */
VALUE eval_ruby_node(NODE * node, VALUE self, VALUE cref)
{
#ifdef RUBY_VM
  if(RTEST(cref))
  {
    rb_raise(
        rb_eArgError,
        "Cannot set cref on YARV");
  }
  else
  {
    VALUE filename = node->nd_file
      ? rb_str_new2(node->nd_file)
      : rb_str_new2("<unknown>");
    VALUE iseq;

    if(nd_type(node) != NODE_SCOPE)
    {
      /* TODO: This is kinda hokey */
      ID * local_tbl = ruby_current_thread->cfp->iseq
        ? ruby_current_thread->cfp->iseq->local_table
        : 0;
      node = NEW_NODE(NODE_SCOPE, local_tbl, node, 0);
    }

    iseq = rb_iseq_new(
        node,
        rb_str_new2("<compiled>"),
        filename,
        self,
        ISEQ_TYPE_TOP);

    /* VALUE str = ruby_iseq_disasm(iseq);
    rb_io_puts(1, &str, rb_stdout); */

    /* VALUE result = rb_iseq_eval(iseq);
    return result; */

    rb_proc_t * p;
    VALUE proc;

    proc = create_proc(rb_cProc, Qnil, iseq_check(iseq));
    GetProcPtr(proc, p);
    p->block.self = self;

    /* TODO: cref */

    return rb_funcall(proc, rb_intern("call"), 0);
  }
#else
  {
    /* Ruby doesn't give us access to rb_eval, so we have to fake it. */
    struct BLOCK * b;
    VALUE proc;

    proc = create_proc(rb_cProc, Qnil, node, 0);
    Data_Get_Struct(proc, struct BLOCK, b);
    b->self = self;

    if(RTEST(cref))
    {
      b->cref = unwrap_node(cref);
    }

    return rb_funcall(proc, rb_intern("call"), 0);
  }
#endif
}
  
/*
 * call-seq:
 *   node.eval(Object) => Object
 *
 * Evaluate a node with the given object as self and returns the result.
 */
static VALUE node_eval(int argc, VALUE * argv, VALUE node)
{
  NODE * n = unwrap_node(node);

  VALUE self;
  VALUE cref = Qnil;
  rb_scan_args(argc, argv, "11", &self, &cref);

  if(rb_safe_level() >= 2)
  {
    /* evaluating a node can cause a crash */
    rb_raise(rb_eSecurityError, "Insecure: can't eval node");
  }

  return eval_ruby_node(n, self, cref);
}

/* ---------------------------------------------------------------------
 * Node marshalling
 * ---------------------------------------------------------------------
 */

void dump_node_to_hash(NODE * n, int node_type, VALUE node_hash)
{
  VALUE s1 = Qnil, s2 = Qnil, s3 = Qnil;
  Node_Type_Descrip const *descrip = node_type_descrip(node_type);
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
#ifdef HAVE_RB_SOURCE_FILENAME
  if(n->nd_file)
  {
    nd_file = rb_str_new2(n->nd_file);
  }
#endif

  arr = rb_ary_new();
  rb_ary_push(arr, INT2NUM(n->flags));
  rb_ary_push(arr, nd_file);
  rb_ary_push(arr, s1);
  rb_ary_push(arr, s2);
  rb_ary_push(arr, s3);

  rb_hash_aset(node_hash, node_id(n), arr);
}

void dump_node_or_iseq_to_hash(VALUE n, int node_type, VALUE node_hash)
{
#ifdef RUBY_VM
  if(TYPE(n) == T_DATA && CLASS_OF(n) == rb_cISeq)
  {
    return dump_iseq_to_hash(n, node_hash);
  }
#endif

  dump_node_to_hash((NODE *)n, node_type, node_hash);
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

#ifdef HAVE_RB_SOURCE_FILENAME
  /* Note that the garbage collector CAN be invoked at this point, so
   * any node object the GC knowns about must be in a consistent state.
   */
  if(rb_nd_file != Qnil)
  {
    Check_Type(rb_nd_file, T_STRING);
    nd_file = rb_source_filename(RSTRING_PTR(rb_nd_file));
  }
#endif

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

#ifdef RUBY_VM
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
  dump_node_to_hash(n, nd_type(n), node_hash);
  return node_hash;
}

#ifndef RUBY_VM
/* From eval.c */
static void
compile_error(at)
    const char *at;
{
    VALUE str;

    ruby_nerrs = 0;
    str = rb_str_buf_new2("compile error");
    if (at) {
        rb_str_buf_cat2(str, " in ");
        rb_str_buf_cat2(str, at);
    }
    rb_str_buf_cat(str, "\n", 1);
    if (!NIL_P(ruby_errinfo)) {
        rb_str_append(str, rb_obj_as_string(ruby_errinfo));
    }
    rb_exc_raise(rb_exc_new3(rb_eSyntaxError, str));
}
#endif

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

#ifdef RUBY_VM
  if(!node)
  {
    rb_exc_raise(GET_THREAD()->errinfo);
  }
#else
  if(ruby_nerrs > 0)
  {
    ruby_nerrs = 0;
    compile_error(0);
  }
#endif

  return wrap_node(node);
}

#ifdef RUBY_VM
/*
 * call-seq:
 *   node.bytecode_compile => VM::InstructionSequence
 *
 * Compile a parsed node tree into a bytecode sequence.
 *
 * @param name the name of the new iseq (default <unknown>)
 * @param filename the filename for the new iseq (default same as name)
 */
static VALUE node_bytecode_compile(int argc, VALUE * argv, VALUE self)
{
  VALUE name = Qnil;
  VALUE filename = Qnil;
  rb_scan_args(argc, argv, "02", &name, &filename);

  if(name == Qnil)
  {
    name = rb_str_new2("<unknown>");
  }

  if(filename == Qnil)
  {
    filename = name;
  }

  NODE * node = unwrap_node(self);

  return rb_iseq_new(
      node,
      name,
      filename,
      Qfalse,
      ISEQ_TYPE_TOP);
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

  if(rb_safe_level() >= 4)
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

  if(   rb_safe_level() >= 4
     || (rb_safe_level() >= 1 && OBJ_TAINTED(str)))
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

#ifndef HAVE_RB_OBJ_RESPOND_TO
static VALUE rb_obj_respond_to(VALUE obj, ID id, int priv)
{
  VALUE include_private = priv ? Qtrue : Qfalse;
  return rb_funcall(obj, rb_intern("respond_to?"), 2, ID2SYM(id), include_private);
}
#endif

/*
 * call-seq:
 *   node.swap(another_node) => Node
 *
 * Swap one node with another.  Both nodes must respond to the #swap
 * method.  Returns the receiver.
 */
static VALUE node_swap(VALUE self, VALUE other)
{
  NODE * n1;
  NODE * n2;
  NODE tmp;

  if(!rb_obj_respond_to(other, rb_intern("swap"), 0))
  {
    rb_raise(rb_eArgError, "Argument must respond to #swap");
  }

  if(   rb_safe_level() >= 4
     || (rb_safe_level() >= 1 && (OBJ_TAINTED(other) || OBJ_TAINTED(self))))
  {
    /* no playing with knives in the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't swap node");
  }

  Data_Get_Struct(self, NODE, n1);
  Data_Get_Struct(other, NODE, n2);

  tmp = *n1;
  *n1 = *n2;
  *n2 = tmp;

  return self;
}

/* ---------------------------------------------------------------------
 * Eval tree
 * ---------------------------------------------------------------------
 */

/* TODO Not quite sure how to get BEGIN nodes on 1.9.x... */
#ifndef RUBY_VM
extern NODE *ruby_eval_tree_begin;

extern NODE *ruby_eval_tree;

static VALUE ruby_eval_tree_begin_getter(ID id, void * data, struct global_entry * entry)
{
  if(rb_safe_level() >= 4)
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

static void ruby_eval_tree_begin_setter(VALUE val, ID id, void * data, struct global_entry * entry)
{
  rb_raise(rb_eNotImpError, "ruby_eval_tree_begin_setter() not implemented");
}

static VALUE ruby_eval_tree_getter(ID id, void * data, struct global_entry * entry)
{
  if(rb_safe_level() >= 4)
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

static void ruby_eval_tree_setter(VALUE val, ID id, void * data, struct global_entry * entry)
{
  rb_raise(rb_eNotImpError, "ruby_eval_tree_setter() not implemented");
}

static VALUE ruby_top_cref_getter(ID id, void * data, struct global_entry * entry)
{
  if(rb_safe_level() >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get top cref");
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

static void ruby_top_cref_setter(VALUE val, ID id, void * data, struct global_entry * entry)
{
  if(rb_safe_level() >= 2)
  {
    rb_raise(rb_eSecurityError, "Insecure: can't set top cref");
  }

  ruby_top_cref = unwrap_node(val);
}

static VALUE ruby_cref_getter(ID id, void * data, struct global_entry * entry)
{
  if(rb_safe_level() >= 4)
  {
    /* no access to potentially sensitive data from the sandbox */
    rb_raise(rb_eSecurityError, "Insecure: can't get current cref");
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

static void ruby_cref_setter(VALUE val, ID id, void * data, struct global_entry * entry)
{
  if(rb_safe_level() >= 2)
  {
    rb_raise(rb_eSecurityError, "Insecure: can't set current cref");
  }

  ruby_cref = unwrap_node(val);
}

#endif

/* ---------------------------------------------------------------------
 * Initialization
 * ---------------------------------------------------------------------
 */

void Init_node(void)
{
  /* This needs to be defined before we require any other files, because
   * we have a circular dependency.
   * (node.so depends on iseq.so depends on module.so depends on
   * node.so)
   */
  rb_cNode = rb_define_class("Node", rb_cObject);

#ifdef RUBY_VM
  rb_require("internal/vm/iseq");
#endif

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
          "This version of Ruby/Internal was built with a different "
          "version of ruby (built with %d, running on %d)",
          RUBY_VERSION_CODE,
          actual_ruby_version_code);
    }
  }

#ifdef HAVE_RB_DEFINE_ALLOC_FUNC
  rb_define_alloc_func(rb_cNode, node_allocate);
#endif

  rb_define_method(rb_cNode, "address", node_address, 0);
  rb_define_method(rb_cNode, "flags", node_flags, 0);
  rb_define_method(rb_cNode, "nd_file", node_nd_file, 0);
  rb_define_method(rb_cNode, "nd_line", node_nd_line, 0);
  rb_define_method(rb_cNode, "nd_type", node_nd_type, 0);
  rb_define_method(rb_cNode, "members", node_members, 0);
  rb_define_method(rb_cNode, "eval", node_eval, -1);
  rb_define_method(rb_cNode, "[]", node_bracket, 1);
  rb_define_method(rb_cNode, "inspect", node_inspect, 0);
  rb_define_singleton_method(rb_cNode, "type", node_s_type, 0);

  rb_define_singleton_method(rb_cNode, "compile_string", node_compile_string, -1);
#ifdef RUBY_VM
  rb_define_method(rb_cNode, "bytecode_compile", node_bytecode_compile, -1);
#endif
  rb_define_method(rb_cNode, "_dump", node_dump, 1);
  rb_define_singleton_method(rb_cNode, "_load", node_load, 1);

  /* TODO: undefine swap for types that are "unsafe" to swap */
  rb_define_method(rb_cNode, "swap", node_swap, 1);

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

  rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));

  wrapped_nodes = rb_hash_new();

  rb_global_variable(&wrapped_nodes);
  rb_set_end_proc(wrapped_nodes_end_proc, Qnil);

#ifndef RUBY_VM
  rb_define_virtual_variable(
      "$ruby_eval_tree_begin",
      ruby_eval_tree_begin_getter,
      ruby_eval_tree_begin_setter);

  rb_define_virtual_variable(
      "$ruby_eval_tree",
      ruby_eval_tree_getter,
      ruby_eval_tree_setter);

  rb_define_virtual_variable(
      "$ruby_top_cref",
      ruby_top_cref_getter,
      ruby_top_cref_setter);

  rb_define_virtual_variable(
      "$ruby_cref",
      ruby_cref_getter,
      ruby_cref_setter);
#endif
}

