#include "nodeinfo.h"
#include "version.h"
#include "global_entry.h"
#include "builtins.h"

static int is_flip_state(ID id)
{
  if(id == 0)
  {
    return 1;
  }
#ifdef HAVE_RB_IS_LOCAL_ID
  if(!rb_is_local_id(id))
  {
    return 1;
  }
#endif
  return 0;
}

static VALUE variable_names(ID * tbl)
{
  if(tbl)
  {
    size_t j;
    VALUE arr = rb_ary_new();
    /* A tbl contains the names of local variables.  The first
     * element is the size of the table.  The next two elements
     * are $_ and $~.  The rest of the elements are the names of
     * the variables themselves.
     */
    for(j = 3; j <= tbl[0]; ++j)
    {
      if(is_flip_state(tbl[j]))
      {
        /* flip state */
        rb_ary_push(arr, Qnil);
      }
      else
      {
        rb_ary_push(arr, ID2SYM(tbl[j]));
      }
    }
    return arr;
  }
  else
  {
    return Qnil;
  }
}

/* Every node contains three elements.  This function takes any one of
 * those elements, converts it to a Ruby object that can be dumped
 * (since nodes can't be dumped), and puts the resulting object into
 * node_hash.
 */
VALUE dump_node_elem(enum Node_Elem_Name nen, NODE * n, VALUE node_hash)
{
  switch(nen)
  {
    case NEN_NONE:
      return Qnil;
    case NEN_1ST:
      if(n->nd_1st)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_1st);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_1st,
          node_type,
          node_hash);
        return node_id(n->nd_1st);
      }
      else
      {
        return Qnil;
      }
    case NEN_2ND:
      if(n->nd_2nd)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_2nd);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_2nd,
          node_type,
          node_hash);
        return node_id(n->nd_2nd);
      }
      else
      {
        return Qnil;
      }
    case NEN_AID:
      if(n->nd_aid == 0)
      {
        return Qfalse;
      }
      else if(n->nd_aid == 1)
      {
        return Qtrue;
      }
      else
      {
        return ID2SYM(n->nd_aid);
      }
    case NEN_ALEN:
      {
      return LONG2NUM((long)n->nd_alen);
      }
    case NEN_ARGC:
      {
      return LONG2NUM((long)n->nd_argc);
      }
    case NEN_ARGS:
      if(n->nd_args)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_args);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_args,
          node_type,
          node_hash);
        return node_id(n->nd_args);
      }
      else
      {
        return Qnil;
      }
    case NEN_BEG:
      if(n->nd_beg)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_beg);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_beg,
          node_type,
          node_hash);
        return node_id(n->nd_beg);
      }
      else
      {
        return Qnil;
      }
    case NEN_BODY:
      if(n->nd_body)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_body);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_body,
          node_type,
          node_hash);
        return node_id(n->nd_body);
      }
      else
      {
        return Qnil;
      }
    case NEN_CFLAG:
      {
      return LONG2NUM((long)n->nd_cflag);
      }
    case NEN_CFNC:
      /* rb_funcall(rb_cObject, rb_intern("pp"), wrap_node(n), 0); */
      rb_raise(rb_eArgError, "Cannot dump cfunc");
    case NEN_CLSS:
      switch(TYPE(n->nd_clss))
      {
        case T_CLASS:
        case T_MODULE:
        {
          /* When dumping a class, we dump just the name (otherwise we'd
           * get multiple copies of the class each time we load a method
           * on the other side).
           */
          VALUE klass = (VALUE)n->nd_clss;
          VALUE path;
          if(FL_TEST(klass, FL_SINGLETON))
          {
            VALUE singleton = rb_iv_get(klass, "__attached__");
            path = rb_class_path(singleton);
          }
          else
          {
            path = rb_class_path(klass);
            if(STR2CSTR(path)[0] == '#')
            {
              rb_raise(rb_eArgError, "cannot dump anonymous class");
            }
          }
          return rb_assoc_new(
              INT2NUM(((struct RBasic *)(n->nd_clss))->flags),
              path);
        }

        case T_NODE:
          rb_raise(
              rb_eRuntimeError,
              "Wrong node elem clss for node type %d",
              nd_type(n));
        
        default:
          /* TODO: would like to dump flags, not type */
          return rb_assoc_new(
              INT2NUM(TYPE((VALUE)n->nd_clss)),
              (VALUE)n->nd_clss);
      }
    case NEN_CNT:
      {
      return LONG2NUM((long)n->nd_cnt);
      }
    case NEN_COND:
      if(n->nd_cond)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_cond);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_cond,
          node_type,
          node_hash);
        return node_id(n->nd_cond);
      }
      else
      {
        return Qnil;
      }
    case NEN_CPATH:
      if(n->nd_cpath)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_cpath);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_cpath,
          node_type,
          node_hash);
        return node_id(n->nd_cpath);
      }
      else
      {
        return Qnil;
      }
    case NEN_CVAL:
      switch(TYPE(n->nd_cval))
      {
        case T_CLASS:
        case T_MODULE:
        {
          /* When dumping a class, we dump just the name (otherwise we'd
           * get multiple copies of the class each time we load a method
           * on the other side).
           */
          VALUE klass = (VALUE)n->nd_cval;
          VALUE path;
          if(FL_TEST(klass, FL_SINGLETON))
          {
            VALUE singleton = rb_iv_get(klass, "__attached__");
            path = rb_class_path(singleton);
          }
          else
          {
            path = rb_class_path(klass);
            if(STR2CSTR(path)[0] == '#')
            {
              rb_raise(rb_eArgError, "cannot dump anonymous class");
            }
          }
          return rb_assoc_new(
              INT2NUM(((struct RBasic *)(n->nd_cval))->flags),
              path);
        }

        case T_NODE:
          rb_raise(
              rb_eRuntimeError,
              "Wrong node elem cval for node type %d",
              nd_type(n));
        
        default:
          /* TODO: would like to dump flags, not type */
          return rb_assoc_new(
              INT2NUM(TYPE((VALUE)n->nd_cval)),
              (VALUE)n->nd_cval);
      }
    case NEN_DEFN:
      if(n->nd_defn)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_defn);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_defn,
          node_type,
          node_hash);
        return node_id(n->nd_defn);
      }
      else
      {
        return Qnil;
      }
    case NEN_ELSE:
      if(n->nd_else)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_else);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_else,
          node_type,
          node_hash);
        return node_id(n->nd_else);
      }
      else
      {
        return Qnil;
      }
    case NEN_END:
      if(n->nd_end)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_end);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_end,
          node_type,
          node_hash);
        return node_id(n->nd_end);
      }
      else
      {
        return Qnil;
      }
    case NEN_ENSR:
      if(n->nd_ensr)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_ensr);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_ensr,
          node_type,
          node_hash);
        return node_id(n->nd_ensr);
      }
      else
      {
        return Qnil;
      }
    case NEN_ENTRY:
      if(n->nd_entry->id == 0)
      {
        return Qfalse;
      }
      else
      {
        return ID2SYM(n->nd_entry->id);
      }
    case NEN_FRML:
      if(n->nd_frml)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_frml);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_frml,
          node_type,
          node_hash);
        return node_id(n->nd_frml);
      }
      else
      {
        return Qnil;
      }
    case NEN_HEAD:
      if(n->nd_head)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_head);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_head,
          node_type,
          node_hash);
        return node_id(n->nd_head);
      }
      else
      {
        return Qnil;
      }
    case NEN_IBDY:
      if(n->nd_ibdy)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_ibdy);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_ibdy,
          node_type,
          node_hash);
        return node_id(n->nd_ibdy);
      }
      else
      {
        return Qnil;
      }
    case NEN_ITER:
      if(n->nd_iter)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_iter);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_iter,
          node_type,
          node_hash);
        return node_id(n->nd_iter);
      }
      else
      {
        return Qnil;
      }
    case NEN_LIT:
      switch(TYPE(n->nd_lit))
      {
        case T_CLASS:
        case T_MODULE:
        {
          /* When dumping a class, we dump just the name (otherwise we'd
           * get multiple copies of the class each time we load a method
           * on the other side).
           */
          VALUE klass = (VALUE)n->nd_lit;
          VALUE path;
          if(FL_TEST(klass, FL_SINGLETON))
          {
            VALUE singleton = rb_iv_get(klass, "__attached__");
            path = rb_class_path(singleton);
          }
          else
          {
            path = rb_class_path(klass);
            if(STR2CSTR(path)[0] == '#')
            {
              rb_raise(rb_eArgError, "cannot dump anonymous class");
            }
          }
          return rb_assoc_new(
              INT2NUM(((struct RBasic *)(n->nd_lit))->flags),
              path);
        }

        case T_NODE:
          rb_raise(
              rb_eRuntimeError,
              "Wrong node elem lit for node type %d",
              nd_type(n));
        
        default:
          /* TODO: would like to dump flags, not type */
          return rb_assoc_new(
              INT2NUM(TYPE((VALUE)n->nd_lit)),
              (VALUE)n->nd_lit);
      }
    case NEN_MID:
      if(n->nd_mid == 0)
      {
        return Qfalse;
      }
      else if(n->nd_mid == 1)
      {
        return Qtrue;
      }
      else
      {
        return ID2SYM(n->nd_mid);
      }
    case NEN_MODL:
      if(n->nd_modl == 0)
      {
        return Qfalse;
      }
      else if(n->nd_modl == 1)
      {
        return Qtrue;
      }
      else
      {
        return ID2SYM(n->nd_modl);
      }
    case NEN_NEXT:
      if(n->nd_next)
      {
        int node_type;
        if(1 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_next);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_next,
          node_type,
          node_hash);
        return node_id(n->nd_next);
      }
      else
      {
        return Qnil;
      }
    case NEN_NOEX:
      {
      return LONG2NUM((long)n->nd_noex);
      }
    case NEN_NTH:
      {
      return LONG2NUM((long)n->nd_nth);
      }
    case NEN_OPT:
      if(n->nd_opt)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_opt);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_opt,
          node_type,
          node_hash);
        return node_id(n->nd_opt);
      }
      else
      {
        return Qnil;
      }
    case NEN_ORIG:
      switch(TYPE(n->nd_orig))
      {
        case T_CLASS:
        case T_MODULE:
        {
          /* When dumping a class, we dump just the name (otherwise we'd
           * get multiple copies of the class each time we load a method
           * on the other side).
           */
          VALUE klass = (VALUE)n->nd_orig;
          VALUE path;
          if(FL_TEST(klass, FL_SINGLETON))
          {
            VALUE singleton = rb_iv_get(klass, "__attached__");
            path = rb_class_path(singleton);
          }
          else
          {
            path = rb_class_path(klass);
            if(STR2CSTR(path)[0] == '#')
            {
              rb_raise(rb_eArgError, "cannot dump anonymous class");
            }
          }
          return rb_assoc_new(
              INT2NUM(((struct RBasic *)(n->nd_orig))->flags),
              path);
        }

        case T_NODE:
          rb_raise(
              rb_eRuntimeError,
              "Wrong node elem orig for node type %d",
              nd_type(n));
        
        default:
          /* TODO: would like to dump flags, not type */
          return rb_assoc_new(
              INT2NUM(TYPE((VALUE)n->nd_orig)),
              (VALUE)n->nd_orig);
      }
    case NEN_RECV:
      if(n->nd_recv)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_recv);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_recv,
          node_type,
          node_hash);
        return node_id(n->nd_recv);
      }
      else
      {
        return Qnil;
      }
    case NEN_RESQ:
      if(n->nd_resq)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_resq);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_resq,
          node_type,
          node_hash);
        return node_id(n->nd_resq);
      }
      else
      {
        return Qnil;
      }
    case NEN_REST:
      if(n->nd_rest)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_rest);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_rest,
          node_type,
          node_hash);
        return node_id(n->nd_rest);
      }
      else
      {
        return Qnil;
      }
    case NEN_RVAL:
      if(n->nd_2nd)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_2nd);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_2nd,
          node_type,
          node_hash);
        return node_id(n->nd_2nd);
      }
      else
      {
        return Qnil;
      }
    case NEN_STATE:
      {
      return LONG2NUM((long)n->nd_state);
      }
    case NEN_STTS:
      if(n->nd_stts)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_stts);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_stts,
          node_type,
          node_hash);
        return node_id(n->nd_stts);
      }
      else
      {
        return Qnil;
      }
    case NEN_SUPER:
      if(n->nd_super)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_super);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_super,
          node_type,
          node_hash);
        return node_id(n->nd_super);
      }
      else
      {
        return Qnil;
      }
    case NEN_TAG:
      if(n->nd_tag == 0)
      {
        return Qfalse;
      }
      else if(n->nd_tag == 1)
      {
        return Qtrue;
      }
      else
      {
        return ID2SYM(n->nd_tag);
      }
    case NEN_TBL:
      {
      VALUE v = variable_names(n->nd_tbl);
      return v;
      }
    case NEN_TVAL:
      switch(TYPE(n->nd_tval))
      {
        case T_CLASS:
        case T_MODULE:
        {
          /* When dumping a class, we dump just the name (otherwise we'd
           * get multiple copies of the class each time we load a method
           * on the other side).
           */
          VALUE klass = (VALUE)n->nd_tval;
          VALUE path;
          if(FL_TEST(klass, FL_SINGLETON))
          {
            VALUE singleton = rb_iv_get(klass, "__attached__");
            path = rb_class_path(singleton);
          }
          else
          {
            path = rb_class_path(klass);
            if(STR2CSTR(path)[0] == '#')
            {
              rb_raise(rb_eArgError, "cannot dump anonymous class");
            }
          }
          return rb_assoc_new(
              INT2NUM(((struct RBasic *)(n->nd_tval))->flags),
              path);
        }

        case T_NODE:
          rb_raise(
              rb_eRuntimeError,
              "Wrong node elem tval for node type %d",
              nd_type(n));
        
        default:
          /* TODO: would like to dump flags, not type */
          return rb_assoc_new(
              INT2NUM(TYPE((VALUE)n->nd_tval)),
              (VALUE)n->nd_tval);
      }
    case NEN_VALUE:
      if(n->nd_value)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_value);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_value,
          node_type,
          node_hash);
        return node_id(n->nd_value);
      }
      else
      {
        return Qnil;
      }
    case NEN_VAR:
      if(n->nd_var)
      {
        int node_type;
        if(0 && nd_type(n) == NODE_OP_ASGN2)
        {
          /* All children of NODE_OP_ASGN2 are NODE_OP_ASGN2_ARG */
          node_type = NODE_OP_ASGN2_ARG;
        }
        else
        {
          node_type = nd_type(n->nd_var);
        }

        dump_node_or_iseq_to_hash(
          (VALUE)n->nd_var,
          node_type,
          node_hash);
        return node_id(n->nd_var);
      }
      else
      {
        return Qnil;
      }
    case NEN_VID:
      if(n->nd_vid == 0)
      {
        return Qfalse;
      }
      else if(n->nd_vid == 1)
      {
        return Qtrue;
      }
      else
      {
        return ID2SYM(n->nd_vid);
      }
  }
  rb_raise(rb_eArgError, "Invalid Node_Elem_Name %d", nen);
}

void load_node_elem(enum Node_Elem_Name nen, VALUE v, NODE * n, VALUE node_hash, VALUE id_hash)
{
  switch(nen)
  {
    case NEN_NONE:
      return;
    case NEN_1ST:
    {
      if(v == Qnil)
      {
        n->nd_1st = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_1st = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_2ND:
    {
      if(v == Qnil)
      {
        n->nd_2nd = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_2nd = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_AID:
    {
      if(v == Qfalse)
      {
        n->nd_aid = 0;
      }
      else if(v == Qfalse)
      {
        n->nd_aid = 1;
      }
      else
      {
        n->nd_aid = SYM2ID(v);
      }
      return;
      }
    case NEN_ALEN:
    {
      n->nd_alen = NUM2LONG(v);
      return;
      }
    case NEN_ARGC:
    {
      n->nd_argc = NUM2LONG(v);
      return;
      }
    case NEN_ARGS:
    {
      if(v == Qnil)
      {
        n->nd_args = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_args = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_BEG:
    {
      if(v == Qnil)
      {
        n->nd_beg = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_beg = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_BODY:
    {
      if(v == Qnil)
      {
        n->nd_body = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_body = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_CFLAG:
    {
      n->nd_cflag = NUM2LONG(v);
      return;
      }
    case NEN_CFNC:
    {
      rb_raise(rb_eRuntimeError, "Cannot load cfunc");
      }
    case NEN_CLSS:
    {
      Check_Type(v, T_ARRAY);
      if(RARRAY_LEN(v) != 2)
      {
        rb_raise(rb_eArgError, "wrong size for array");
      }
      int flags = NUM2INT(RARRAY_PTR(v)[0]);
      switch(flags & T_MASK)
      {
        case T_CLASS:
        case T_MODULE:
        {
          VALUE str = RARRAY_PTR(v)[1];
          Check_Type(str, T_STRING);
          VALUE klass = rb_path2class(STR2CSTR(str));
          if(flags & FL_SINGLETON)
          {
            *(VALUE *)(&n->nd_clss) =
              rb_singleton_class(klass);
          }
          else
          {
            *(VALUE *)(&n->nd_clss) =
              klass;
          }
          break;
        }
        default:
          *(VALUE *)(&n->nd_clss) = RARRAY_PTR(v)[1];
          break;
      }
      return;
      }
    case NEN_CNT:
    {
      n->nd_cnt = NUM2LONG(v);
      return;
      }
    case NEN_COND:
    {
      if(v == Qnil)
      {
        n->nd_cond = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_cond = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_CPATH:
    {
      if(v == Qnil)
      {
        n->nd_cpath = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_cpath = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_CVAL:
    {
      Check_Type(v, T_ARRAY);
      if(RARRAY_LEN(v) != 2)
      {
        rb_raise(rb_eArgError, "wrong size for array");
      }
      int flags = NUM2INT(RARRAY_PTR(v)[0]);
      switch(flags & T_MASK)
      {
        case T_CLASS:
        case T_MODULE:
        {
          VALUE str = RARRAY_PTR(v)[1];
          Check_Type(str, T_STRING);
          VALUE klass = rb_path2class(STR2CSTR(str));
          if(flags & FL_SINGLETON)
          {
            *(VALUE *)(&n->nd_cval) =
              rb_singleton_class(klass);
          }
          else
          {
            *(VALUE *)(&n->nd_cval) =
              klass;
          }
          break;
        }
        default:
          *(VALUE *)(&n->nd_cval) = RARRAY_PTR(v)[1];
          break;
      }
      return;
      }
    case NEN_DEFN:
    {
      if(v == Qnil)
      {
        n->nd_defn = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_defn = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_ELSE:
    {
      if(v == Qnil)
      {
        n->nd_else = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_else = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_END:
    {
      if(v == Qnil)
      {
        n->nd_end = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_end = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_ENSR:
    {
      if(v == Qnil)
      {
        n->nd_ensr = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_ensr = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_ENTRY:
    {
      n->nd_entry = rb_global_entry(SYM2ID(v));
      return;
      }
    case NEN_FRML:
    {
      if(v == Qnil)
      {
        n->nd_frml = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_frml = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_HEAD:
    {
      if(v == Qnil)
      {
        n->nd_head = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_head = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_IBDY:
    {
      if(v == Qnil)
      {
        n->nd_ibdy = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_ibdy = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_ITER:
    {
      if(v == Qnil)
      {
        n->nd_iter = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_iter = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_LIT:
    {
      Check_Type(v, T_ARRAY);
      if(RARRAY_LEN(v) != 2)
      {
        rb_raise(rb_eArgError, "wrong size for array");
      }
      int flags = NUM2INT(RARRAY_PTR(v)[0]);
      switch(flags & T_MASK)
      {
        case T_CLASS:
        case T_MODULE:
        {
          VALUE str = RARRAY_PTR(v)[1];
          Check_Type(str, T_STRING);
          VALUE klass = rb_path2class(STR2CSTR(str));
          if(flags & FL_SINGLETON)
          {
            *(VALUE *)(&n->nd_lit) =
              rb_singleton_class(klass);
          }
          else
          {
            *(VALUE *)(&n->nd_lit) =
              klass;
          }
          break;
        }
        default:
          *(VALUE *)(&n->nd_lit) = RARRAY_PTR(v)[1];
          break;
      }
      return;
      }
    case NEN_MID:
    {
      if(v == Qfalse)
      {
        n->nd_mid = 0;
      }
      else if(v == Qfalse)
      {
        n->nd_mid = 1;
      }
      else
      {
        n->nd_mid = SYM2ID(v);
      }
      return;
      }
    case NEN_MODL:
    {
      if(v == Qfalse)
      {
        n->nd_modl = 0;
      }
      else if(v == Qfalse)
      {
        n->nd_modl = 1;
      }
      else
      {
        n->nd_modl = SYM2ID(v);
      }
      return;
      }
    case NEN_NEXT:
    {
      if(v == Qnil)
      {
        n->nd_next = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_next = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_NOEX:
    {
      n->nd_noex = NUM2LONG(v);
      return;
      }
    case NEN_NTH:
    {
      n->nd_nth = NUM2LONG(v);
      return;
      }
    case NEN_OPT:
    {
      if(v == Qnil)
      {
        n->nd_opt = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_opt = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_ORIG:
    {
      Check_Type(v, T_ARRAY);
      if(RARRAY_LEN(v) != 2)
      {
        rb_raise(rb_eArgError, "wrong size for array");
      }
      int flags = NUM2INT(RARRAY_PTR(v)[0]);
      switch(flags & T_MASK)
      {
        case T_CLASS:
        case T_MODULE:
        {
          VALUE str = RARRAY_PTR(v)[1];
          Check_Type(str, T_STRING);
          VALUE klass = rb_path2class(STR2CSTR(str));
          if(flags & FL_SINGLETON)
          {
            *(VALUE *)(&n->nd_orig) =
              rb_singleton_class(klass);
          }
          else
          {
            *(VALUE *)(&n->nd_orig) =
              klass;
          }
          break;
        }
        default:
          *(VALUE *)(&n->nd_orig) = RARRAY_PTR(v)[1];
          break;
      }
      return;
      }
    case NEN_RECV:
    {
      if(v == Qnil)
      {
        n->nd_recv = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_recv = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_RESQ:
    {
      if(v == Qnil)
      {
        n->nd_resq = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_resq = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_REST:
    {
      if(v == Qnil)
      {
        n->nd_rest = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_rest = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_RVAL:
    {
      if(v == Qnil)
      {
        n->nd_2nd = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_2nd = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_STATE:
    {
      n->nd_state = NUM2LONG(v);
      return;
      }
    case NEN_STTS:
    {
      if(v == Qnil)
      {
        n->nd_stts = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_stts = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_SUPER:
    {
      if(v == Qnil)
      {
        n->nd_super = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_super = id_to_node(nid);
        }
        else
        {
          n->u3.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u3.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u3.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_TAG:
    {
      if(v == Qfalse)
      {
        n->nd_tag = 0;
      }
      else if(v == Qfalse)
      {
        n->nd_tag = 1;
      }
      else
      {
        n->nd_tag = SYM2ID(v);
      }
      return;
      }
    case NEN_TBL:
    {
      if(v == Qnil)
      {
        n->nd_tbl = 0;
      }
      else
      {
        size_t len, j;
        ID * tmp_tbl;

        Check_Type(v, T_ARRAY);
        len = RARRAY_LEN(v);
        tmp_tbl = ALLOCA_N(ID, len + 3);
        tmp_tbl[0] = (ID)(len + 2);
        tmp_tbl[1] = (ID)('_');
        tmp_tbl[2] = (ID)('~');
        for(j = 0; j < len; ++j)
        {
          VALUE e = RARRAY_PTR(v)[j];
          if(e == Qnil)
          {
            /* flip state */
            tmp_tbl[j + 3] = 0;
          }
          else
          {
            tmp_tbl[j + 3] = SYM2ID(e);
          }
        }
        n->nd_tbl = ALLOC_N(ID, len + 3);
        memcpy(n->nd_tbl, tmp_tbl, sizeof(ID) * (len + 3));
      }
      return;
      }
    case NEN_TVAL:
    {
      Check_Type(v, T_ARRAY);
      if(RARRAY_LEN(v) != 2)
      {
        rb_raise(rb_eArgError, "wrong size for array");
      }
      int flags = NUM2INT(RARRAY_PTR(v)[0]);
      switch(flags & T_MASK)
      {
        case T_CLASS:
        case T_MODULE:
        {
          VALUE str = RARRAY_PTR(v)[1];
          Check_Type(str, T_STRING);
          VALUE klass = rb_path2class(STR2CSTR(str));
          if(flags & FL_SINGLETON)
          {
            *(VALUE *)(&n->nd_tval) =
              rb_singleton_class(klass);
          }
          else
          {
            *(VALUE *)(&n->nd_tval) =
              klass;
          }
          break;
        }
        default:
          *(VALUE *)(&n->nd_tval) = RARRAY_PTR(v)[1];
          break;
      }
      return;
      }
    case NEN_VALUE:
    {
      if(v == Qnil)
      {
        n->nd_value = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_value = id_to_node(nid);
        }
        else
        {
          n->u2.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u2.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u2.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_VAR:
    {
      if(v == Qnil)
      {
        n->nd_var = 0;
      }
      else
      {
        VALUE nid = rb_hash_aref(id_hash, v);
        if(RTEST(nid))
        {
          n->nd_var = id_to_node(nid);
        }
        else
        {
          n->u1.node = (NODE *)load_node_or_iseq_from_hash(
              v, node_hash, id_hash);
          if(nd_type(n->u1.node) == NODE_OP_ASGN2_ARG)
          {
            nd_set_type(n->u1.node, NODE_OP_ASGN2);
          }
        }
      }
      return;
      }
    case NEN_VID:
    {
      if(v == Qfalse)
      {
        n->nd_vid = 0;
      }
      else if(v == Qfalse)
      {
        n->nd_vid = 1;
      }
      else
      {
        n->nd_vid = SYM2ID(v);
      }
      return;
      }
  }
  rb_raise(rb_eRuntimeError, "Internal error: invalid Node_Elem_Name %d", nen);
}

/*
 * Return the Node's _1st_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_1st(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_1st) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_1st,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_1st);
    }
  }
  else
  {
    return (VALUE)n->nd_1st;
  }
}
/*
 * Return the Node's _2nd_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_2nd(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_2nd) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_2nd,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_2nd);
    }
  }
  else
  {
    return (VALUE)n->nd_2nd;
  }
}
/*
 * Return the Node's _aid_ member.  The return type is a
 * Symbol.
 */
static VALUE node_aid(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  if(n->nd_aid == 0)
  {
    return Qfalse;
  }
  else if(n->nd_aid == 1)
  {
    return Qtrue;
  }
  else
  {
    return ID2SYM(n->nd_aid);
  }
}
/*
 * Return the Node's _alen_ member.  The return type is an
 * Integer.
 */
static VALUE node_alen(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_alen);
}
/*
 * Return the Node's _argc_ member.  The return type is an
 * Integer.
 */
static VALUE node_argc(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_argc);
}
/*
 * Return the Node's _args_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_args(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_args) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_args,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_args);
    }
  }
  else
  {
    return (VALUE)n->nd_args;
  }
}
/*
 * Return the Node's _beg_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_beg(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_beg) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_beg,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_beg);
    }
  }
  else
  {
    return (VALUE)n->nd_beg;
  }
}
/*
 * Return the Node's _body_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_body(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_body) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_body,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_body);
    }
  }
  else
  {
    return (VALUE)n->nd_body;
  }
}
/*
 * Return the Node's _cflag_ member.  The return type is an
 * Integer.
 */
static VALUE node_cflag(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_cflag);
}
static VALUE node_cfnc(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM((long)(n->nd_cfnc));
}
/*
 * Return the Node's _clss_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_clss(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_clss) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_clss,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_clss);
    }
  }
  else
  {
    return (VALUE)n->nd_clss;
  }
}
/*
 * Return the Node's _cnt_ member.  The return type is an
 * Integer.
 */
static VALUE node_cnt(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_cnt);
}
/*
 * Return the Node's _cond_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_cond(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_cond) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_cond,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_cond);
    }
  }
  else
  {
    return (VALUE)n->nd_cond;
  }
}
/*
 * Return the Node's _cpath_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_cpath(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_cpath) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_cpath,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_cpath);
    }
  }
  else
  {
    return (VALUE)n->nd_cpath;
  }
}
/*
 * Return the Node's _cval_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_cval(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_cval) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_cval,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_cval);
    }
  }
  else
  {
    return (VALUE)n->nd_cval;
  }
}
/*
 * Return the Node's _defn_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_defn(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_defn) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_defn,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_defn);
    }
  }
  else
  {
    return (VALUE)n->nd_defn;
  }
}
/*
 * Return the Node's _else_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_else(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_else) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_else,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_else);
    }
  }
  else
  {
    return (VALUE)n->nd_else;
  }
}
/*
 * Return the Node's _end_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_end(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_end) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_end,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_end);
    }
  }
  else
  {
    return (VALUE)n->nd_end;
  }
}
/*
 * Return the Node's _ensr_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_ensr(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_ensr) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_ensr,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_ensr);
    }
  }
  else
  {
    return (VALUE)n->nd_ensr;
  }
}
/*
 * Return the Node's _entry_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_entry(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  if(n->nd_entry->id == 0)
  {
    return Qfalse;
  }
  else
  {
    return ID2SYM(n->nd_entry->id);
  }
}
/*
 * Return the Node's _head_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_head(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_head) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_head,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_head);
    }
  }
  else
  {
    return (VALUE)n->nd_head;
  }
}
/*
 * Return the Node's _iter_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_iter(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_iter) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_iter,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_iter);
    }
  }
  else
  {
    return (VALUE)n->nd_iter;
  }
}
/*
 * Return the Node's _lit_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_lit(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_lit) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_lit,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_lit);
    }
  }
  else
  {
    return (VALUE)n->nd_lit;
  }
}
/*
 * Return the Node's _mid_ member.  The return type is a
 * Symbol.
 */
static VALUE node_mid(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  if(n->nd_mid == 0)
  {
    return Qfalse;
  }
  else if(n->nd_mid == 1)
  {
    return Qtrue;
  }
  else
  {
    return ID2SYM(n->nd_mid);
  }
}
/*
 * Return the Node's _next_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_next(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_next) == T_NODE)
  {
    if(1 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_next,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_next);
    }
  }
  else
  {
    return (VALUE)n->nd_next;
  }
}
/*
 * Return the Node's _noex_ member.  The return type is an
 * Integer.
 */
static VALUE node_noex(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_noex);
}
/*
 * Return the Node's _nth_ member.  The return type is an
 * Integer.
 */
static VALUE node_nth(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_nth);
}
/*
 * Return the Node's _opt_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_opt(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_opt) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_opt,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_opt);
    }
  }
  else
  {
    return (VALUE)n->nd_opt;
  }
}
/*
 * Return the Node's _orig_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_orig(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_orig) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_orig,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_orig);
    }
  }
  else
  {
    return (VALUE)n->nd_orig;
  }
}
/*
 * Return the Node's _recv_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_recv(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_recv) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_recv,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_recv);
    }
  }
  else
  {
    return (VALUE)n->nd_recv;
  }
}
/*
 * Return the Node's _resq_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_resq(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_resq) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_resq,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_resq);
    }
  }
  else
  {
    return (VALUE)n->nd_resq;
  }
}
/*
 * Return the Node's _rest_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_rest(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_rest) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_rest,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_rest);
    }
  }
  else
  {
    return (VALUE)n->nd_rest;
  }
}
/*
 * Return the Node's _rval_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_rval(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_rval) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_rval,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_rval);
    }
  }
  else
  {
    return (VALUE)n->nd_rval;
  }
}
/*
 * Return the Node's _state_ member.  The return type is an
 * Integer.
 */
static VALUE node_state(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return LONG2NUM(n->nd_state);
}
/*
 * Return the Node's _stts_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_stts(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_stts) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_stts,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_stts);
    }
  }
  else
  {
    return (VALUE)n->nd_stts;
  }
}
/*
 * Return the Node's _super_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_super(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_super) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_super,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_super);
    }
  }
  else
  {
    return (VALUE)n->nd_super;
  }
}
/*
 * Return the Node's _tbl_ member.  The return value is an
 * Array holding names of variables.
 */
static VALUE node_tbl(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  return variable_names(n->nd_tbl);
}
/*
 * Return the Node's _tval_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_tval(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_tval) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_tval,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_tval);
    }
  }
  else
  {
    return (VALUE)n->nd_tval;
  }
}
/*
 * Return the Node's _value_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_value(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_value) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_value,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_value);
    }
  }
  else
  {
    return (VALUE)n->nd_value;
  }
}
/*
 * Return the Node's _var_ member.  The return type is
 * either a Node or an Object.
 */
static VALUE node_var(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);

  if(TYPE(n->nd_var) == T_NODE)
  {
    if(0 && nd_type(n) == NODE_OP_ASGN2)
    {
      return wrap_node_as(
        (NODE *)n->nd_var,
        rb_cNodeSubclass[NODE_OP_ASGN2_ARG]);
    }
    else
    {
      return wrap_node((NODE *)n->nd_var);
    }
  }
  else
  {
    return (VALUE)n->nd_var;
  }
}
/*
 * Return the Node's _vid_ member.  The return type is a
 * Symbol.
 */
static VALUE node_vid(VALUE self)
{
  NODE * n;
  Data_Get_Struct(self, NODE, n);
  if(n->nd_vid == 0)
  {
    return Qfalse;
  }
  else if(n->nd_vid == 1)
  {
    return Qtrue;
  }
  else
  {
    return ID2SYM(n->nd_vid);
  }
}

/*
 * Return an array of strings containing the names of the node class's
 * members.
 */
VALUE node_s_members(VALUE klass)
{
  return rb_iv_get(klass, "__member__");
}


void define_node_subclass_methods()
{
  /* For rdoc: rb_cNode = rb_define_class("Node", rb_cObject); */
  VALUE rb_cNode = rb_const_get(rb_cObject, rb_intern("Node"));
  VALUE members;
  int j;

  for(j = 0; j < NODE_LAST; ++j)
  {
    rb_cNodeSubclass[j] = Qnil;
  }


  /* Document-class: Node::ALIAS
   * Represents an alias expression of the form:
   *   alias 1st 2nd
   * where 2nd is the name of an existing method and 1st is the name of
   * its new alias.
   */
  {
    VALUE rb_cALIAS = rb_define_class_under(rb_cNode, "ALIAS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ALIAS] = rb_cALIAS;
    rb_iv_set(rb_cALIAS, "__member__", members);
    rb_iv_set(rb_cALIAS, "__type__", INT2NUM(NODE_ALIAS));
    rb_define_singleton_method(rb_cALIAS, "members", node_s_members, 0);
    rb_define_method(rb_cALIAS, "first", node_1st, 0);
    rb_ary_push(members, rb_str_new2("first"));
    rb_define_method(rb_cALIAS, "second", node_2nd, 0);
    rb_ary_push(members, rb_str_new2("second"));
  }

  /* Document-class: Node::ALLOCA
   * A node used for temporary allocation of memory on platforms that do
   * not support alloca.
   * 
   * It should never be evaluated as an expression.
   */
#ifdef HAVE_NODE_ALLOCA
  {
    VALUE rb_cALLOCA = rb_define_class_under(rb_cNode, "ALLOCA", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ALLOCA] = rb_cALLOCA;
    rb_iv_set(rb_cALLOCA, "__member__", members);
    rb_iv_set(rb_cALLOCA, "__type__", INT2NUM(NODE_ALLOCA));
    rb_define_singleton_method(rb_cALLOCA, "members", node_s_members, 0);

    /* Document-method: cfnc
     * a pointer to the allocated memory
     */
    rb_define_method(rb_cALLOCA, "cfnc", node_cfnc, 0);
    rb_ary_push(members, rb_str_new2("cfnc"));

    /* Document-method: value
     * a pointer to the previously allocated temporary node
     */
    rb_define_method(rb_cALLOCA, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: cnt
     * the number of bytes allocated
     */
    rb_define_method(rb_cALLOCA, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }
#endif

  /* Document-class: Node::AND
   * Represents a logical 'and' of the form:
   *   first && second
   * The expression will short-circuit and yield the result of the left
   * hand side if it is false or nil, else it will evaluate the right
   * hand side and use it as the result of the expression.
   */
  {
    VALUE rb_cAND = rb_define_class_under(rb_cNode, "AND", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_AND] = rb_cAND;
    rb_iv_set(rb_cAND, "__member__", members);
    rb_iv_set(rb_cAND, "__type__", INT2NUM(NODE_AND));
    rb_define_singleton_method(rb_cAND, "members", node_s_members, 0);

    /* Document-method: first
     * the expression on the left hand side
     */
    rb_define_method(rb_cAND, "first", node_1st, 0);
    rb_ary_push(members, rb_str_new2("first"));

    /* Document-method: second
     * the expression on the right hand side
     */
    rb_define_method(rb_cAND, "second", node_2nd, 0);
    rb_ary_push(members, rb_str_new2("second"));
  }

  /* Document-class: Node::ARGS
   * Represents the arguments to a method.
   */
  {
    VALUE rb_cARGS = rb_define_class_under(rb_cNode, "ARGS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ARGS] = rb_cARGS;
    rb_iv_set(rb_cARGS, "__member__", members);
    rb_iv_set(rb_cARGS, "__type__", INT2NUM(NODE_ARGS));
    rb_define_singleton_method(rb_cARGS, "members", node_s_members, 0);

    /* Document-method: rest
     * an assignment node to assign the rest arg, if it is present
     */
    rb_define_method(rb_cARGS, "rest", node_rest, 0);
    rb_ary_push(members, rb_str_new2("rest"));

    /* Document-method: opt
     * a list of assignment nodes to assign default values to the
     * optional arguments if no argument is specified
     */
    rb_define_method(rb_cARGS, "opt", node_opt, 0);
    rb_ary_push(members, rb_str_new2("opt"));

    /* Document-method: cnt
     * the number of required arguments
     */
    rb_define_method(rb_cARGS, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::ARGSCAT
   * Represents the concatenation of a list of arguments and a splatted
   * value, e.g.:
   *   a, b, *value
   * Evaluates head to create an array.  Evaluates body and performs a
   * splat operation on the result to create another array (see SPLAT).
   * Concatenates the the second array onto the end of the first to
   * produce the result.
   */
  {
    VALUE rb_cARGSCAT = rb_define_class_under(rb_cNode, "ARGSCAT", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ARGSCAT] = rb_cARGSCAT;
    rb_iv_set(rb_cARGSCAT, "__member__", members);
    rb_iv_set(rb_cARGSCAT, "__type__", INT2NUM(NODE_ARGSCAT));
    rb_define_singleton_method(rb_cARGSCAT, "members", node_s_members, 0);

    /* Document-method: head
     * a list of fixed arguments
     */
    rb_define_method(rb_cARGSCAT, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: body
     * the last argument, which will be splatted onto the end of the
     * fixed arguments
     */
    rb_define_method(rb_cARGSCAT, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::ARGSPUSH
   * Represents the splat portion of the index in an expression:
   *   obj[arg, arg, *head] = body
   * Evaluates head to create an array.  Evaluates body and pushes the
   * result onto the end of the array, which will then be used to form
   * the arguments of the method call.
   */
  {
    VALUE rb_cARGSPUSH = rb_define_class_under(rb_cNode, "ARGSPUSH", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ARGSPUSH] = rb_cARGSPUSH;
    rb_iv_set(rb_cARGSPUSH, "__member__", members);
    rb_iv_set(rb_cARGSPUSH, "__type__", INT2NUM(NODE_ARGSPUSH));
    rb_define_singleton_method(rb_cARGSPUSH, "members", node_s_members, 0);
    rb_define_method(rb_cARGSPUSH, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
    rb_define_method(rb_cARGSPUSH, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::ARRAY
   * Represents an array of elements.  Evaluation of this node creates a
   * new Array by evalating the given expressions and placing them into
   * the array.
   */
  {
    VALUE rb_cARRAY = rb_define_class_under(rb_cNode, "ARRAY", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ARRAY] = rb_cARRAY;
    rb_iv_set(rb_cARRAY, "__member__", members);
    rb_iv_set(rb_cARRAY, "__type__", INT2NUM(NODE_ARRAY));
    rb_define_singleton_method(rb_cARRAY, "members", node_s_members, 0);

    /* Document-method: head
     * the first element of the array
     */
    rb_define_method(rb_cARRAY, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
    rb_define_method(rb_cARRAY, "alen", node_alen, 0);
    rb_ary_push(members, rb_str_new2("alen"));

    /* Document-method: next
     * the tail of the array
     */
    rb_define_method(rb_cARRAY, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::ATTRASGN
   * Represents attribute assignment in the form:
   *   recv.mid = args
   * or:
   *   recv.mid=(args).
   */
  {
    VALUE rb_cATTRASGN = rb_define_class_under(rb_cNode, "ATTRASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ATTRASGN] = rb_cATTRASGN;
    rb_iv_set(rb_cATTRASGN, "__member__", members);
    rb_iv_set(rb_cATTRASGN, "__type__", INT2NUM(NODE_ATTRASGN));
    rb_define_singleton_method(rb_cATTRASGN, "members", node_s_members, 0);

    /* Document-method: args
     * the arguments to the method
     */
    rb_define_method(rb_cATTRASGN, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));

    /* Document-method: mid
     * the id of the attribute, with a trailing '=' sign
     */
    rb_define_method(rb_cATTRASGN, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));

    /* Document-method: recv
     * the receiver of the method
     */
    rb_define_method(rb_cATTRASGN, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::ATTRSET
   * A placeholder for an attribute writer method, which can added to a
   * class by using attr_writer:
   *   attr_writer :attribute
   * Its reader counterpart is IVAR.
   */
  {
    VALUE rb_cATTRSET = rb_define_class_under(rb_cNode, "ATTRSET", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ATTRSET] = rb_cATTRSET;
    rb_iv_set(rb_cATTRSET, "__member__", members);
    rb_iv_set(rb_cATTRSET, "__type__", INT2NUM(NODE_ATTRSET));
    rb_define_singleton_method(rb_cATTRSET, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the attribute, with a leading '@' sign
     */
    rb_define_method(rb_cATTRSET, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::BACK_REF
   * Represents one of the regex back reference variables:
   * * $& - last match
   * * $` - pre
   * * $' - post
   * * $+ - last
   */
  {
    VALUE rb_cBACK_REF = rb_define_class_under(rb_cNode, "BACK_REF", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BACK_REF] = rb_cBACK_REF;
    rb_iv_set(rb_cBACK_REF, "__member__", members);
    rb_iv_set(rb_cBACK_REF, "__type__", INT2NUM(NODE_BACK_REF));
    rb_define_singleton_method(rb_cBACK_REF, "members", node_s_members, 0);

    /* Document-method: nth
     * the integer representation of the character of the variable to
     * reference, one of '&', '`', '\'', or '+'
     */
    rb_define_method(rb_cBACK_REF, "nth", node_nth, 0);
    rb_ary_push(members, rb_str_new2("nth"));

    /* Document-method: cnt
     * the index into the local variable table where the match data is
     * stored
     */
    rb_define_method(rb_cBACK_REF, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::BEGIN
   * Represents a begin/end block.
   * 
   * TODO: Need an example
   */
  {
    VALUE rb_cBEGIN = rb_define_class_under(rb_cNode, "BEGIN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BEGIN] = rb_cBEGIN;
    rb_iv_set(rb_cBEGIN, "__member__", members);
    rb_iv_set(rb_cBEGIN, "__type__", INT2NUM(NODE_BEGIN));
    rb_define_singleton_method(rb_cBEGIN, "members", node_s_members, 0);
    rb_define_method(rb_cBEGIN, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::BLOCK
   * Represents a block of code (a succession of multiple expressions).
   * A single block node can hold two expressions: one expression to be
   * evaluated and second expression, which may be another BLOCK.
   * The first node in the block may be of type ARGS, in which case it
   * represents the arguments to the current method.
   * The second node in the block may be of type BLOCK_ARG, in which case
   * it represents an explicit block argument.
   * The result of the block is the last expression evaluated.
   */
  {
    VALUE rb_cBLOCK = rb_define_class_under(rb_cNode, "BLOCK", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BLOCK] = rb_cBLOCK;
    rb_iv_set(rb_cBLOCK, "__member__", members);
    rb_iv_set(rb_cBLOCK, "__type__", INT2NUM(NODE_BLOCK));
    rb_define_singleton_method(rb_cBLOCK, "members", node_s_members, 0);

    /* Document-method: head
     * the first expression in the block of code
     */
    rb_define_method(rb_cBLOCK, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: next
     * the second expression in the block of code
     */
    rb_define_method(rb_cBLOCK, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::BLOCK_ARG
   * Represents a block argument in a method definition, e.g.:
   *   def foo(&arg)
   *   end
   */
  {
    VALUE rb_cBLOCK_ARG = rb_define_class_under(rb_cNode, "BLOCK_ARG", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BLOCK_ARG] = rb_cBLOCK_ARG;
    rb_iv_set(rb_cBLOCK_ARG, "__member__", members);
    rb_iv_set(rb_cBLOCK_ARG, "__type__", INT2NUM(NODE_BLOCK_ARG));
    rb_define_singleton_method(rb_cBLOCK_ARG, "members", node_s_members, 0);

    /* Document-method: cnt
     * the index into the local variable table of the name of the block
     * argument, not including the implicit variables.
     */
    rb_define_method(rb_cBLOCK_ARG, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::BLOCK_PASS
   * Represents an object being passed to a method as a block, e.g.:
   *   foo(&bar)
   * 
   * The body node is first evaluates to get an object, then #to_proc to
   * convert it into a proc if necessary.  The iter node is then
   * evaluated to call a method, passing the proc as a block parameter to
   * the method.
   */
  {
    VALUE rb_cBLOCK_PASS = rb_define_class_under(rb_cNode, "BLOCK_PASS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BLOCK_PASS] = rb_cBLOCK_PASS;
    rb_iv_set(rb_cBLOCK_PASS, "__member__", members);
    rb_iv_set(rb_cBLOCK_PASS, "__type__", INT2NUM(NODE_BLOCK_PASS));
    rb_define_singleton_method(rb_cBLOCK_PASS, "members", node_s_members, 0);

    /* Document-method: body
     * the object to pass as a block
     */
    rb_define_method(rb_cBLOCK_PASS, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: iter
     * an expression which calls a method
     */
    rb_define_method(rb_cBLOCK_PASS, "iter", node_iter, 0);
    rb_ary_push(members, rb_str_new2("iter"));
  }

  /* Document-class: Node::BMETHOD
   * A placeholder for a method defined using define_method, e.g.:
   *   define_method(:foo) {
   *     ...
   *   }
   * 
   * See also DMETHOD.
   */
  {
    VALUE rb_cBMETHOD = rb_define_class_under(rb_cNode, "BMETHOD", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BMETHOD] = rb_cBMETHOD;
    rb_iv_set(rb_cBMETHOD, "__member__", members);
    rb_iv_set(rb_cBMETHOD, "__type__", INT2NUM(NODE_BMETHOD));
    rb_define_singleton_method(rb_cBMETHOD, "members", node_s_members, 0);

    /* Document-method: cval
     * the Proc object passed to define_method, which contains the body
     * of the method
     */
    rb_define_method(rb_cBMETHOD, "cval", node_cval, 0);
    rb_ary_push(members, rb_str_new2("cval"));
  }

  /* Document-class: Node::BREAK
   * Represents the 'break' keyword.  Causes control to be transferred
   * out of the current loop.
   */
  {
    VALUE rb_cBREAK = rb_define_class_under(rb_cNode, "BREAK", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_BREAK] = rb_cBREAK;
    rb_iv_set(rb_cBREAK, "__member__", members);
    rb_iv_set(rb_cBREAK, "__type__", INT2NUM(NODE_BREAK));
    rb_define_singleton_method(rb_cBREAK, "members", node_s_members, 0);

    /* Document-method: stts
     * the value to be used as the "return" value of the loop, 0 if  nil is to be used.
     */
    rb_define_method(rb_cBREAK, "stts", node_stts, 0);
    rb_ary_push(members, rb_str_new2("stts"));
  }

  /* Document-class: Node::CALL
   * Represents a method call in the form recv.mid(args).
   */
  {
    VALUE rb_cCALL = rb_define_class_under(rb_cNode, "CALL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CALL] = rb_cCALL;
    rb_iv_set(rb_cCALL, "__member__", members);
    rb_iv_set(rb_cCALL, "__type__", INT2NUM(NODE_CALL));
    rb_define_singleton_method(rb_cCALL, "members", node_s_members, 0);

    /* Document-method: args
     * the arguments to the method
     */
    rb_define_method(rb_cCALL, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));

    /* Document-method: mid
     * the method id
     */
    rb_define_method(rb_cCALL, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));

    /* Document-method: recv
     * the receiver of the method
     */
    rb_define_method(rb_cCALL, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::CASE
   * Represents the value portion of a case/end block, e.g.:
   *   case head
   *   body
   *   end
   *   next
   */
  {
    VALUE rb_cCASE = rb_define_class_under(rb_cNode, "CASE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CASE] = rb_cCASE;
    rb_iv_set(rb_cCASE, "__member__", members);
    rb_iv_set(rb_cCASE, "__type__", INT2NUM(NODE_CASE));
    rb_define_singleton_method(rb_cCASE, "members", node_s_members, 0);

    /* Document-method: head
     * the value to select on
     */
    rb_define_method(rb_cCASE, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: body
     * a linked list of nodes, each node representing a when
     * conditional
     */
    rb_define_method(rb_cCASE, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
    rb_define_method(rb_cCASE, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::CDECL
   * Represents constant assignment of the form:
   *   vid = value
   */
  {
    VALUE rb_cCDECL = rb_define_class_under(rb_cNode, "CDECL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CDECL] = rb_cCDECL;
    rb_iv_set(rb_cCDECL, "__member__", members);
    rb_iv_set(rb_cCDECL, "__type__", INT2NUM(NODE_CDECL));
    rb_define_singleton_method(rb_cCDECL, "members", node_s_members, 0);

    /* Document-method: value
     * the value to be assigned to the constant
     */
    rb_define_method(rb_cCDECL, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the constant to be assigned, all uppercase
     */
    rb_define_method(rb_cCDECL, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::CFUNC
   * A placeholder for a function implemented in C.
   */
  {
    VALUE rb_cCFUNC = rb_define_class_under(rb_cNode, "CFUNC", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CFUNC] = rb_cCFUNC;
    rb_iv_set(rb_cCFUNC, "__member__", members);
    rb_iv_set(rb_cCFUNC, "__type__", INT2NUM(NODE_CFUNC));
    rb_define_singleton_method(rb_cCFUNC, "members", node_s_members, 0);
    rb_define_method(rb_cCFUNC, "cfnc", node_cfnc, 0);
    rb_ary_push(members, rb_str_new2("cfnc"));
    rb_define_method(rb_cCFUNC, "argc", node_argc, 0);
    rb_ary_push(members, rb_str_new2("argc"));
  }

  /* Document-class: Node::CLASS
   * Represents a class definition, e.g.:
   *   class cpath
   *     body
   *   end
   * 
   * or:
   *   class cpath < super
   *     body
   *   end
   * 
   * The class definition is evaluated in a new lexical scope.
   * 
   * The result of the expression is the last expression evaluated in the
   * body.
   */
  {
    VALUE rb_cCLASS = rb_define_class_under(rb_cNode, "CLASS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CLASS] = rb_cCLASS;
    rb_iv_set(rb_cCLASS, "__member__", members);
    rb_iv_set(rb_cCLASS, "__type__", INT2NUM(NODE_CLASS));
    rb_define_singleton_method(rb_cCLASS, "members", node_s_members, 0);

    /* Document-method: cpath
     * the name of the class to define
     */
    rb_define_method(rb_cCLASS, "cpath", node_cpath, 0);
    rb_ary_push(members, rb_str_new2("cpath"));

    /* Document-method: body
     * the body of the class definition
     */
    rb_define_method(rb_cCLASS, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: super
     * an expression returning the base class, or false if there is no
     * base class specified
     */
    rb_define_method(rb_cCLASS, "super", node_super, 0);
    rb_ary_push(members, rb_str_new2("super"));
  }

  /* Document-class: Node::COLON2
   * Represents a constant lookup in a particular class.  This expression
   * has the form:
   *   klass::mid
   * 
   * where klass is the result of evaluating the expression in the head
   * member.
   */
  {
    VALUE rb_cCOLON2 = rb_define_class_under(rb_cNode, "COLON2", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_COLON2] = rb_cCOLON2;
    rb_iv_set(rb_cCOLON2, "__member__", members);
    rb_iv_set(rb_cCOLON2, "__type__", INT2NUM(NODE_COLON2));
    rb_define_singleton_method(rb_cCOLON2, "members", node_s_members, 0);

    /* Document-method: head
     * an expression specifying the class in which to do the lookup
     */
    rb_define_method(rb_cCOLON2, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: mid
     * the name of the method or constant to call/look up
     */
    rb_define_method(rb_cCOLON2, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));
  }

  /* Document-class: Node::COLON3
   * Represents a constant lookup or method call in class Object.  This
   * expression has the form:
   *   ::mid
   */
  {
    VALUE rb_cCOLON3 = rb_define_class_under(rb_cNode, "COLON3", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_COLON3] = rb_cCOLON3;
    rb_iv_set(rb_cCOLON3, "__member__", members);
    rb_iv_set(rb_cCOLON3, "__type__", INT2NUM(NODE_COLON3));
    rb_define_singleton_method(rb_cCOLON3, "members", node_s_members, 0);

    /* Document-method: mid
     * the name of the method or constant to call/look up
     */
    rb_define_method(rb_cCOLON3, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));
  }

  /* Document-class: Node::CONST
   * Represents a constant lookup in the current class.  The current
   * class is the class in which the containing scope was defined.  The
   * result of the expression is the value of the constant.
   */
  {
    VALUE rb_cCONST = rb_define_class_under(rb_cNode, "CONST", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CONST] = rb_cCONST;
    rb_iv_set(rb_cCONST, "__member__", members);
    rb_iv_set(rb_cCONST, "__type__", INT2NUM(NODE_CONST));
    rb_define_singleton_method(rb_cCONST, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the constant to look up
     */
    rb_define_method(rb_cCONST, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::CREF
   * A temporary node used to store the value of ruby_cref or
   * ruby_top_cref and later restore it.  The cref holds a reference to
   * the cbase, which, among other things, is used for constant and class
   * variable lookup.
   * 
   * It should never be evaluated as an expression.
   */
  {
    VALUE rb_cCREF = rb_define_class_under(rb_cNode, "CREF", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CREF] = rb_cCREF;
    rb_iv_set(rb_cCREF, "__member__", members);
    rb_iv_set(rb_cCREF, "__type__", INT2NUM(NODE_CREF));
    rb_define_singleton_method(rb_cCREF, "members", node_s_members, 0);

    /* Document-method: body
     * always 0 (false)
     */
    rb_define_method(rb_cCREF, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: next
     * the previous value of ruby_cref or ruby_top_cref
     */
    rb_define_method(rb_cCREF, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));

    /* Document-method: clss
     * the new class to use for the cbase.
     */
    rb_define_method(rb_cCREF, "clss", node_clss, 0);
    rb_ary_push(members, rb_str_new2("clss"));
  }

  /* Document-class: Node::CVAR
   * Represents a class variable retrieval.  The result of the expression
   * is the value of the class variable.
   */
  {
    VALUE rb_cCVAR = rb_define_class_under(rb_cNode, "CVAR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CVAR] = rb_cCVAR;
    rb_iv_set(rb_cCVAR, "__member__", members);
    rb_iv_set(rb_cCVAR, "__type__", INT2NUM(NODE_CVAR));
    rb_define_singleton_method(rb_cCVAR, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the class variable, with two leading '@' characters.
     */
    rb_define_method(rb_cCVAR, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::CVASGN
   * Represents class variable assignment in a method context.
   */
  {
    VALUE rb_cCVASGN = rb_define_class_under(rb_cNode, "CVASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CVASGN] = rb_cCVASGN;
    rb_iv_set(rb_cCVASGN, "__member__", members);
    rb_iv_set(rb_cCVASGN, "__type__", INT2NUM(NODE_CVASGN));
    rb_define_singleton_method(rb_cCVASGN, "members", node_s_members, 0);

    /* Document-method: value
     * an expression whose result is the new value of the class
     * variable
     */
    rb_define_method(rb_cCVASGN, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the class variable to assign
     */
    rb_define_method(rb_cCVASGN, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::CVDECL
   * Represents class variable assignment in a class context.
   * 
   * A warning is emitted if the variable already exists.
   */
  {
    VALUE rb_cCVDECL = rb_define_class_under(rb_cNode, "CVDECL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_CVDECL] = rb_cCVDECL;
    rb_iv_set(rb_cCVDECL, "__member__", members);
    rb_iv_set(rb_cCVDECL, "__type__", INT2NUM(NODE_CVDECL));
    rb_define_singleton_method(rb_cCVDECL, "members", node_s_members, 0);

    /* Document-method: value
     * an expression whose result is the new value of the class
     * variable
     */
    rb_define_method(rb_cCVDECL, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the class variable to assign
     */
    rb_define_method(rb_cCVDECL, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::DASGN
   * Represents dynamic local variable assignment.  Dynamic assignment
   * differs from static assignment in that the slots for static local
   * variables are allocated when the method is called, wereas slots for
   * dynamic variables are allocated when the variable is first assigned
   * to.  When searching for the variable, dynamic assignment searches
   * backward up the stack to see if the variable exists in any previous
   * block in the current frame; if it does, it assigns to the slot found
   * in that block, otherwise it creates a new variable slot.  As a
   * result, dynamic assignment is typically much slower than static
   * assignment.
   */
  {
    VALUE rb_cDASGN = rb_define_class_under(rb_cNode, "DASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DASGN] = rb_cDASGN;
    rb_iv_set(rb_cDASGN, "__member__", members);
    rb_iv_set(rb_cDASGN, "__type__", INT2NUM(NODE_DASGN));
    rb_define_singleton_method(rb_cDASGN, "members", node_s_members, 0);

    /* Document-method: value
     * the value to assign to the local variable
     */
    rb_define_method(rb_cDASGN, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the local variable
     */
    rb_define_method(rb_cDASGN, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::DASGN_CURR
   * Represents dynamic local variable assignment in the current frame.
   * See DASGN for a description of how dynamic assignment works.
   */
  {
    VALUE rb_cDASGN_CURR = rb_define_class_under(rb_cNode, "DASGN_CURR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DASGN_CURR] = rb_cDASGN_CURR;
    rb_iv_set(rb_cDASGN_CURR, "__member__", members);
    rb_iv_set(rb_cDASGN_CURR, "__type__", INT2NUM(NODE_DASGN_CURR));
    rb_define_singleton_method(rb_cDASGN_CURR, "members", node_s_members, 0);

    /* Document-method: value
     * the value to assign to the local variable
     */
    rb_define_method(rb_cDASGN_CURR, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the local variable
     */
    rb_define_method(rb_cDASGN_CURR, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::DEFINED
   * Represents an expression using the 'defined?' keyword.  The result
   * is either nil or a string describing the expression.
   */
  {
    VALUE rb_cDEFINED = rb_define_class_under(rb_cNode, "DEFINED", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DEFINED] = rb_cDEFINED;
    rb_iv_set(rb_cDEFINED, "__member__", members);
    rb_iv_set(rb_cDEFINED, "__type__", INT2NUM(NODE_DEFINED));
    rb_define_singleton_method(rb_cDEFINED, "members", node_s_members, 0);

    /* Document-method: head
     * the expression to inspect
     */
    rb_define_method(rb_cDEFINED, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
  }

  /* Document-class: Node::DEFN
   * Represents a method definition, e.g.:
   *   def mid
   *     defn
   *   end
   */
  {
    VALUE rb_cDEFN = rb_define_class_under(rb_cNode, "DEFN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DEFN] = rb_cDEFN;
    rb_iv_set(rb_cDEFN, "__member__", members);
    rb_iv_set(rb_cDEFN, "__type__", INT2NUM(NODE_DEFN));
    rb_define_singleton_method(rb_cDEFN, "members", node_s_members, 0);

    /* Document-method: defn
     * the body of the method definition
     */
    rb_define_method(rb_cDEFN, "defn", node_defn, 0);
    rb_ary_push(members, rb_str_new2("defn"));

    /* Document-method: mid
     * the name of the method* defn the body of the method
     */
    rb_define_method(rb_cDEFN, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));

    /* Document-method: noex
     * the flags which should be used to define the method
     */
    rb_define_method(rb_cDEFN, "noex", node_noex, 0);
    rb_ary_push(members, rb_str_new2("noex"));
  }

  /* Document-class: Node::DEFS
   * Represents a singleton method definition, e.g.:
   *   def recv.mid
   *     defn
   *   end
   */
  {
    VALUE rb_cDEFS = rb_define_class_under(rb_cNode, "DEFS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DEFS] = rb_cDEFS;
    rb_iv_set(rb_cDEFS, "__member__", members);
    rb_iv_set(rb_cDEFS, "__type__", INT2NUM(NODE_DEFS));
    rb_define_singleton_method(rb_cDEFS, "members", node_s_members, 0);
    rb_define_method(rb_cDEFS, "defn", node_defn, 0);
    rb_ary_push(members, rb_str_new2("defn"));

    /* Document-method: mid
     * the name of the method* defn the body of the method
     */
    rb_define_method(rb_cDEFS, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));

    /* Document-method: recv
     * the object to whose singleton class the new method is to be  added
     */
    rb_define_method(rb_cDEFS, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::DMETHOD
   * A placeholder for a method defined using define_method where the
   * passed block is created from a Method object, e.g.:
   *   define_method(:name, method(:foo))
   * 
   * See also BMETHOD.
   */
  {
    VALUE rb_cDMETHOD = rb_define_class_under(rb_cNode, "DMETHOD", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DMETHOD] = rb_cDMETHOD;
    rb_iv_set(rb_cDMETHOD, "__member__", members);
    rb_iv_set(rb_cDMETHOD, "__type__", INT2NUM(NODE_DMETHOD));
    rb_define_singleton_method(rb_cDMETHOD, "members", node_s_members, 0);

    /* Document-method: cval
     * the Method object passed to define_method, which contains the
     * body of the method
     */
    rb_define_method(rb_cDMETHOD, "cval", node_cval, 0);
    rb_ary_push(members, rb_str_new2("cval"));
  }

  /* Document-class: Node::DOT2
   * Represents a range created with the form:
   *   beg..end
   * Creates a range which does not exclude the range end.
   */
  {
    VALUE rb_cDOT2 = rb_define_class_under(rb_cNode, "DOT2", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DOT2] = rb_cDOT2;
    rb_iv_set(rb_cDOT2, "__member__", members);
    rb_iv_set(rb_cDOT2, "__type__", INT2NUM(NODE_DOT2));
    rb_define_singleton_method(rb_cDOT2, "members", node_s_members, 0);

    /* Document-method: beg
     * the beginning of the range
     */
    rb_define_method(rb_cDOT2, "beg", node_beg, 0);
    rb_ary_push(members, rb_str_new2("beg"));

    /* Document-method: end
     * the end of the range
     */
    rb_define_method(rb_cDOT2, "end", node_end, 0);
    rb_ary_push(members, rb_str_new2("end"));
    rb_define_method(rb_cDOT2, "state", node_state, 0);
    rb_ary_push(members, rb_str_new2("state"));
  }

  /* Document-class: Node::DOT3
   * Represents a range created with the form:
   *   beg...end
   * Creates a range which excludes the range end.
   */
  {
    VALUE rb_cDOT3 = rb_define_class_under(rb_cNode, "DOT3", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DOT3] = rb_cDOT3;
    rb_iv_set(rb_cDOT3, "__member__", members);
    rb_iv_set(rb_cDOT3, "__type__", INT2NUM(NODE_DOT3));
    rb_define_singleton_method(rb_cDOT3, "members", node_s_members, 0);

    /* Document-method: beg
     * the beginning of the range
     */
    rb_define_method(rb_cDOT3, "beg", node_beg, 0);
    rb_ary_push(members, rb_str_new2("beg"));

    /* Document-method: end
     * the end of the range
     */
    rb_define_method(rb_cDOT3, "end", node_end, 0);
    rb_ary_push(members, rb_str_new2("end"));
    rb_define_method(rb_cDOT3, "state", node_state, 0);
    rb_ary_push(members, rb_str_new2("state"));
  }

  /* Document-class: Node::DREGX
   * Represents a regular expresion with interpolation.  The node is
   * evaluated by duplicating the regex stored in the 'lit' element, then
   * iterating over the nodes stored in the 'next' element.  Each node
   * found should evaluate to a string, and each resulting string is
   * appended onto the original string.  Interpolation is represented
   * with the EVSTR node.
   */
  {
    VALUE rb_cDREGX = rb_define_class_under(rb_cNode, "DREGX", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DREGX] = rb_cDREGX;
    rb_iv_set(rb_cDREGX, "__member__", members);
    rb_iv_set(rb_cDREGX, "__type__", INT2NUM(NODE_DREGX));
    rb_define_singleton_method(rb_cDREGX, "members", node_s_members, 0);

    /* Document-method: lit
     * a string
     */
    rb_define_method(rb_cDREGX, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));

    /* Document-method: cflag
     * a bitfield containing the options used in the regular
     * expression.  Valid values include:
     * * RE_OPTION_IGNORECASE
     * * RE_OPTION_EXTENDED
     * * RE_OPTION_MULTILINE
     * * RE_OPTION_SINGLELINE
     * * RE_OPTION_LONGEST
     * * RE_MAY_IGNORECASE
     * * RE_OPTIMIZE_ANCHOR
     * * RE_OPTIMIZE_EXACTN
     * * RE_OPTIMIZE_NO_BM
     * * RE_OPTIMIZE_BMATCH
     */
    rb_define_method(rb_cDREGX, "cflag", node_cflag, 0);
    rb_ary_push(members, rb_str_new2("cflag"));

    /* Document-method: next
     * a list of expressions to be appended onto the string
     */
    rb_define_method(rb_cDREGX, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::DREGX_ONCE
   * Represents a regular expression with interpolation with the 'once'
   * flag set.  The regular expression is only interpolated the first
   * time it is encountered.
   */
  {
    VALUE rb_cDREGX_ONCE = rb_define_class_under(rb_cNode, "DREGX_ONCE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DREGX_ONCE] = rb_cDREGX_ONCE;
    rb_iv_set(rb_cDREGX_ONCE, "__member__", members);
    rb_iv_set(rb_cDREGX_ONCE, "__type__", INT2NUM(NODE_DREGX_ONCE));
    rb_define_singleton_method(rb_cDREGX_ONCE, "members", node_s_members, 0);

    /* Document-method: lit
     * a string
     */
    rb_define_method(rb_cDREGX_ONCE, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));

    /* Document-method: cflag
     * a bitfield containing the options used in the regular
     * expression.  See DREGX for a list of valid values.
     */
    rb_define_method(rb_cDREGX_ONCE, "cflag", node_cflag, 0);
    rb_ary_push(members, rb_str_new2("cflag"));

    /* Document-method: next
     * a list of expressions to be appended onto the string
     */
    rb_define_method(rb_cDREGX_ONCE, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::DSTR
   * Represents a string object with interpolation.  The node is
   * evaluated by duplicating the string stored in the 'lit' element,
   * then iterating over the nodes stored in the 'next' element.  Each
   * node found should evalate to a string, and each resulting string is
   * appended onto the regex.  Interpolation is represented with the
   * EVSTR node.
   */
  {
    VALUE rb_cDSTR = rb_define_class_under(rb_cNode, "DSTR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DSTR] = rb_cDSTR;
    rb_iv_set(rb_cDSTR, "__member__", members);
    rb_iv_set(rb_cDSTR, "__type__", INT2NUM(NODE_DSTR));
    rb_define_singleton_method(rb_cDSTR, "members", node_s_members, 0);

    /* Document-method: lit
     * a string
     */
    rb_define_method(rb_cDSTR, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));

    /* Document-method: next
     * a list of expressions to be appended onto the string
     */
    rb_define_method(rb_cDSTR, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::DSYM
   * Represents a symbol with interpolation, e.g.:
   *   :"#{next}"
   * 
   * The node is evaluated by duplicating the string stored in the 'lit'
   * element, then iterating over the nodes stored in the 'next' element.
   * Each node found should evalate to a string, and each resulting
   * string is appended onto the original string.  The final resulting
   * string is then converted into a symbol.  Interpolation is
   * represented with the EVSTR node.
   */
  {
    VALUE rb_cDSYM = rb_define_class_under(rb_cNode, "DSYM", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DSYM] = rb_cDSYM;
    rb_iv_set(rb_cDSYM, "__member__", members);
    rb_iv_set(rb_cDSYM, "__type__", INT2NUM(NODE_DSYM));
    rb_define_singleton_method(rb_cDSYM, "members", node_s_members, 0);
    rb_define_method(rb_cDSYM, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));
    rb_define_method(rb_cDSYM, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::DVAR
   * Represents dynamic local variable retrieval.  See also DASGN.
   */
  {
    VALUE rb_cDVAR = rb_define_class_under(rb_cNode, "DVAR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DVAR] = rb_cDVAR;
    rb_iv_set(rb_cDVAR, "__member__", members);
    rb_iv_set(rb_cDVAR, "__type__", INT2NUM(NODE_DVAR));
    rb_define_singleton_method(rb_cDVAR, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the local variable to retrieve.
     */
    rb_define_method(rb_cDVAR, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::DXSTR
   * Represents a string object with interpolation inside backticks,
   * e.g.:
   *   `lit#{next}`
   * 
   * The node is evaluated by duplicating the string stored in the 'lit'
   * element, then iterating over the nodes stored in the 'next' element.
   * Each node found should evalate to an string, and each resulting
   * string is appended onto the original string.
   * 
   * The resulting string is executed in a subshell and the output from
   * its stdout stored in a string, which becomes the result of the
   * expression.  Interpolation is represented with the EVSTR node.
   */
  {
    VALUE rb_cDXSTR = rb_define_class_under(rb_cNode, "DXSTR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_DXSTR] = rb_cDXSTR;
    rb_iv_set(rb_cDXSTR, "__member__", members);
    rb_iv_set(rb_cDXSTR, "__type__", INT2NUM(NODE_DXSTR));
    rb_define_singleton_method(rb_cDXSTR, "members", node_s_members, 0);

    /* Document-method: lit
     * a string
     */
    rb_define_method(rb_cDXSTR, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));

    /* Document-method: next
     * a list of expressions to be appended onto the string
     */
    rb_define_method(rb_cDXSTR, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::ENSURE
   * Represents an ensure expression, e.g.:
   *   begin
   *     head
   *   ensure
   *     ensr
   *   end
   * 
   * The head expression is first evaluated.  After the expression is
   * evaluated, whether or not the expression raises an exception, the
   * ensr expression is evaluated.
   * 
   * See also RESCUE and RESBDOY.
   */
  {
    VALUE rb_cENSURE = rb_define_class_under(rb_cNode, "ENSURE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ENSURE] = rb_cENSURE;
    rb_iv_set(rb_cENSURE, "__member__", members);
    rb_iv_set(rb_cENSURE, "__type__", INT2NUM(NODE_ENSURE));
    rb_define_singleton_method(rb_cENSURE, "members", node_s_members, 0);

    /* Document-method: head
     * the expression to protect
     */
    rb_define_method(rb_cENSURE, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: ensr
     * the expression to evaluate after the head is evaluated
     */
    rb_define_method(rb_cENSURE, "ensr", node_ensr, 0);
    rb_ary_push(members, rb_str_new2("ensr"));
  }

  /* Document-class: Node::EVSTR
   * Represents a single string interpolation.
   * 
   * Evaluates the given expression and converts its result to a string
   * with #to_s.
   */
  {
    VALUE rb_cEVSTR = rb_define_class_under(rb_cNode, "EVSTR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_EVSTR] = rb_cEVSTR;
    rb_iv_set(rb_cEVSTR, "__member__", members);
    rb_iv_set(rb_cEVSTR, "__type__", INT2NUM(NODE_EVSTR));
    rb_define_singleton_method(rb_cEVSTR, "members", node_s_members, 0);

    /* Document-method: body
     * an expression to evaluate
     */
    rb_define_method(rb_cEVSTR, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::FALSE
   * Represents the keyword 'false'.
   */
  {
    VALUE rb_cFALSE = rb_define_class_under(rb_cNode, "FALSE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_FALSE] = rb_cFALSE;
    rb_iv_set(rb_cFALSE, "__member__", members);
    rb_iv_set(rb_cFALSE, "__type__", INT2NUM(NODE_FALSE));
    rb_define_singleton_method(rb_cFALSE, "members", node_s_members, 0);
  }

  /* Document-class: Node::FBODY
   * A placeholder for a method alias, which can be added to a class by
   * using alias or alias_method:
   *   alias old new
   */
  {
    VALUE rb_cFBODY = rb_define_class_under(rb_cNode, "FBODY", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_FBODY] = rb_cFBODY;
    rb_iv_set(rb_cFBODY, "__member__", members);
    rb_iv_set(rb_cFBODY, "__type__", INT2NUM(NODE_FBODY));
    rb_define_singleton_method(rb_cFBODY, "members", node_s_members, 0);

    /* Document-method: head
     * the method body
     */
    rb_define_method(rb_cFBODY, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: orig
     * the origin class
     */
    rb_define_method(rb_cFBODY, "orig", node_orig, 0);
    rb_ary_push(members, rb_str_new2("orig"));

    /* Document-method: mid
     * the name of the method
     */
    rb_define_method(rb_cFBODY, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));
  }

  /* Document-class: Node::FCALL
   * Represents a method call in the form:
   *   mid(args).
   * 
   * This makes a method call on an object using self as the implicit
   * receiver.
   */
  {
    VALUE rb_cFCALL = rb_define_class_under(rb_cNode, "FCALL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_FCALL] = rb_cFCALL;
    rb_iv_set(rb_cFCALL, "__member__", members);
    rb_iv_set(rb_cFCALL, "__type__", INT2NUM(NODE_FCALL));
    rb_define_singleton_method(rb_cFCALL, "members", node_s_members, 0);

    /* Document-method: args
     * the arguments to the method
     */
    rb_define_method(rb_cFCALL, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));

    /* Document-method: mid
     * the method id
     */
    rb_define_method(rb_cFCALL, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));
  }

  /* Document-class: Node::FLIP2
   * Represents part of an awk-like flip-flop expression of the form:
   *   if beg..end then
   *     body
   *   end
   */
  {
    VALUE rb_cFLIP2 = rb_define_class_under(rb_cNode, "FLIP2", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_FLIP2] = rb_cFLIP2;
    rb_iv_set(rb_cFLIP2, "__member__", members);
    rb_iv_set(rb_cFLIP2, "__type__", INT2NUM(NODE_FLIP2));
    rb_define_singleton_method(rb_cFLIP2, "members", node_s_members, 0);

    /* Document-method: beg
     * the beginning of the range
     */
    rb_define_method(rb_cFLIP2, "beg", node_beg, 0);
    rb_ary_push(members, rb_str_new2("beg"));

    /* Document-method: end
     * the end of the range
     */
    rb_define_method(rb_cFLIP2, "end", node_end, 0);
    rb_ary_push(members, rb_str_new2("end"));

    /* Document-method: cnt
     * the index into the local variable table of the special variable
     * to use in the flip-flop expression (usually 2 for $_)
     */
    rb_define_method(rb_cFLIP2, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::FLIP3
   * Represents part of a sed-like flip-flop expression of the form:
   *   if beg..end then
   *     body
   *   end
   */
  {
    VALUE rb_cFLIP3 = rb_define_class_under(rb_cNode, "FLIP3", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_FLIP3] = rb_cFLIP3;
    rb_iv_set(rb_cFLIP3, "__member__", members);
    rb_iv_set(rb_cFLIP3, "__type__", INT2NUM(NODE_FLIP3));
    rb_define_singleton_method(rb_cFLIP3, "members", node_s_members, 0);

    /* Document-method: beg
     * the beginning of the range
     */
    rb_define_method(rb_cFLIP3, "beg", node_beg, 0);
    rb_ary_push(members, rb_str_new2("beg"));

    /* Document-method: end
     * the end of the range
     */
    rb_define_method(rb_cFLIP3, "end", node_end, 0);
    rb_ary_push(members, rb_str_new2("end"));

    /* Document-method: cnt
     * the index into the local variable table of the special variable
     * to use in the flip-flop expression (usually 2 for $_)
     */
    rb_define_method(rb_cFLIP3, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::FOR
   * Represents a loop constructed with the 'for' keyword, e.g.:
   *   for var in iter do
   *     body
   *   end
   * 
   * This is equivalent to:
   *   iter.each do |*args|
   *   assign args to var
   *   body
   * end
   * 
   * Except that a new block is not created.
   */
  {
    VALUE rb_cFOR = rb_define_class_under(rb_cNode, "FOR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_FOR] = rb_cFOR;
    rb_iv_set(rb_cFOR, "__member__", members);
    rb_iv_set(rb_cFOR, "__type__", INT2NUM(NODE_FOR));
    rb_define_singleton_method(rb_cFOR, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the loop
     */
    rb_define_method(rb_cFOR, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: iter
     * the sequence over which to iterate
     */
    rb_define_method(rb_cFOR, "iter", node_iter, 0);
    rb_ary_push(members, rb_str_new2("iter"));

    /* Document-method: var
     * an assignment node which assigns the next value in the sequence
     * to a variable, which may or may not be local.  May also be a
     * multiple assignment.
     */
    rb_define_method(rb_cFOR, "var", node_var, 0);
    rb_ary_push(members, rb_str_new2("var"));
  }

  /* Document-class: Node::GASGN
   * Represents global variable assignment.
   */
  {
    VALUE rb_cGASGN = rb_define_class_under(rb_cNode, "GASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_GASGN] = rb_cGASGN;
    rb_iv_set(rb_cGASGN, "__member__", members);
    rb_iv_set(rb_cGASGN, "__type__", INT2NUM(NODE_GASGN));
    rb_define_singleton_method(rb_cGASGN, "members", node_s_members, 0);

    /* Document-method: value
     * an expression whose result is the new value of the global variable
     */
    rb_define_method(rb_cGASGN, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the global variable, with a leading '$' character.
     */
    rb_define_method(rb_cGASGN, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
    rb_define_method(rb_cGASGN, "entry", node_entry, 0);
    rb_ary_push(members, rb_str_new2("entry"));
  }

  /* Document-class: Node::GVAR
   * Represents global variable retrieval.
   */
  {
    VALUE rb_cGVAR = rb_define_class_under(rb_cNode, "GVAR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_GVAR] = rb_cGVAR;
    rb_iv_set(rb_cGVAR, "__member__", members);
    rb_iv_set(rb_cGVAR, "__type__", INT2NUM(NODE_GVAR));
    rb_define_singleton_method(rb_cGVAR, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the global variable to retrieve, with a leading '$'
     */
    rb_define_method(rb_cGVAR, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
    rb_define_method(rb_cGVAR, "entry", node_entry, 0);
    rb_ary_push(members, rb_str_new2("entry"));
  }

  /* Document-class: Node::HASH
   * Represents a hash table.  Evaluation of this node creates a new Hash
   * by evaluating the given expressions and placing them into the table.
   */
  {
    VALUE rb_cHASH = rb_define_class_under(rb_cNode, "HASH", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_HASH] = rb_cHASH;
    rb_iv_set(rb_cHASH, "__member__", members);
    rb_iv_set(rb_cHASH, "__type__", INT2NUM(NODE_HASH));
    rb_define_singleton_method(rb_cHASH, "members", node_s_members, 0);

    /* Document-method: head
     * an array of successive keys and values
     */
    rb_define_method(rb_cHASH, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
  }

  /* Document-class: Node::IASGN
   * Represents instance variable assignment.
   */
  {
    VALUE rb_cIASGN = rb_define_class_under(rb_cNode, "IASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_IASGN] = rb_cIASGN;
    rb_iv_set(rb_cIASGN, "__member__", members);
    rb_iv_set(rb_cIASGN, "__type__", INT2NUM(NODE_IASGN));
    rb_define_singleton_method(rb_cIASGN, "members", node_s_members, 0);

    /* Document-method: value
     * the value to assign to the instance variable
     */
    rb_define_method(rb_cIASGN, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the instance variable, with a leading '@' sign
     */
    rb_define_method(rb_cIASGN, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::IF
   * Represents a conditional of the form:
   *   if cond then
   *     body
   *   end
   * 
   * or the form:
   *   if cond then
   *     body
   *   else
   *     else
   *   end
   * 
   * The condition is first evaluated.  If it is true, then body is
   * evaluated, otherwise else is evaluated.  The result is the value of
   * the expression evaluated, or nil if there was no expression
   * present.
   * 
   * A conditional block using elsif has another IF node as the else
   * expression.
   */
  {
    VALUE rb_cIF = rb_define_class_under(rb_cNode, "IF", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_IF] = rb_cIF;
    rb_iv_set(rb_cIF, "__member__", members);
    rb_iv_set(rb_cIF, "__type__", INT2NUM(NODE_IF));
    rb_define_singleton_method(rb_cIF, "members", node_s_members, 0);

    /* Document-method: cond
     * the condition to evaluate
     */
    rb_define_method(rb_cIF, "cond", node_cond, 0);
    rb_ary_push(members, rb_str_new2("cond"));

    /* Document-method: body
     * the expression to evaluate if the expression is true, or false
     * if the expression is empty
     */
    rb_define_method(rb_cIF, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: else
     * the expression to evaluate if the expression is false, or false
     * if the expression is empty
     */
    rb_define_method(rb_cIF, "else", node_else, 0);
    rb_ary_push(members, rb_str_new2("else"));
  }

  /* Document-class: Node::IFUNC
   * A temporary node used in iteration.
   */
  {
    VALUE rb_cIFUNC = rb_define_class_under(rb_cNode, "IFUNC", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_IFUNC] = rb_cIFUNC;
    rb_iv_set(rb_cIFUNC, "__member__", members);
    rb_iv_set(rb_cIFUNC, "__type__", INT2NUM(NODE_IFUNC));
    rb_define_singleton_method(rb_cIFUNC, "members", node_s_members, 0);

    /* Document-method: cfnc
     * a pointer to the C function to which to yield
     */
    rb_define_method(rb_cIFUNC, "cfnc", node_cfnc, 0);
    rb_ary_push(members, rb_str_new2("cfnc"));

    /* Document-method: tval
     * the user-specified data to be passed as the second argument to
     * cfnc
     */
    rb_define_method(rb_cIFUNC, "tval", node_tval, 0);
    rb_ary_push(members, rb_str_new2("tval"));

    /* Document-method: state
     * always 0
     */
    rb_define_method(rb_cIFUNC, "state", node_state, 0);
    rb_ary_push(members, rb_str_new2("state"));
  }

  /* Document-class: Node::ITER
   * Represents an iteration loop, e.g.:
   *   iter do |*args|
   *     assign args to var
   *     body
   *   end
   * 
   * A new block is created so that dynamic variables created inside the
   * loop do not persist once the loop has terminated.
   * 
   * If the iter node is a POSTEXE node, indicates that the expression
   * should be evaluated when the program terminates.
   */
  {
    VALUE rb_cITER = rb_define_class_under(rb_cNode, "ITER", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ITER] = rb_cITER;
    rb_iv_set(rb_cITER, "__member__", members);
    rb_iv_set(rb_cITER, "__type__", INT2NUM(NODE_ITER));
    rb_define_singleton_method(rb_cITER, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the loop
     */
    rb_define_method(rb_cITER, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: iter
     * an expression which calls the desired iteration method, usually
     * recv.each
     */
    rb_define_method(rb_cITER, "iter", node_iter, 0);
    rb_ary_push(members, rb_str_new2("iter"));

    /* Document-method: var
     * an assignment node which assigns the next value in the sequence
     * to a variable, which may or may not be local.  May also be a
     * multiple assignment.
     */
    rb_define_method(rb_cITER, "var", node_var, 0);
    rb_ary_push(members, rb_str_new2("var"));
  }

  /* Document-class: Node::IVAR
   * A placeholder for an attribute reader method, which can added to a
   * class by using attr_reader:
   *   attr_reader :attribute
   * Its writer counterpart is ATTRSET.
   */
  {
    VALUE rb_cIVAR = rb_define_class_under(rb_cNode, "IVAR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_IVAR] = rb_cIVAR;
    rb_iv_set(rb_cIVAR, "__member__", members);
    rb_iv_set(rb_cIVAR, "__type__", INT2NUM(NODE_IVAR));
    rb_define_singleton_method(rb_cIVAR, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the attribute, with a leading '@' sign
     */
    rb_define_method(rb_cIVAR, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
  }

  /* Document-class: Node::LASGN
   * Represents local variable assignment.
   */
  {
    VALUE rb_cLASGN = rb_define_class_under(rb_cNode, "LASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_LASGN] = rb_cLASGN;
    rb_iv_set(rb_cLASGN, "__member__", members);
    rb_iv_set(rb_cLASGN, "__type__", INT2NUM(NODE_LASGN));
    rb_define_singleton_method(rb_cLASGN, "members", node_s_members, 0);

    /* Document-method: value
     * the value to assign to the local variable
     */
    rb_define_method(rb_cLASGN, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: vid
     * the name of the local variable
     */
    rb_define_method(rb_cLASGN, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
    rb_define_method(rb_cLASGN, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::LIT
   * Represents a literal object.  The result of the expression is the
   * object contained in this node.
   */
  {
    VALUE rb_cLIT = rb_define_class_under(rb_cNode, "LIT", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_LIT] = rb_cLIT;
    rb_iv_set(rb_cLIT, "__member__", members);
    rb_iv_set(rb_cLIT, "__type__", INT2NUM(NODE_LIT));
    rb_define_singleton_method(rb_cLIT, "members", node_s_members, 0);

    /* Document-method: lit
     * the object
     */
    rb_define_method(rb_cLIT, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));
  }

  /* Document-class: Node::LVAR
   * Represents local variable retrieval.
   */
  {
    VALUE rb_cLVAR = rb_define_class_under(rb_cNode, "LVAR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_LVAR] = rb_cLVAR;
    rb_iv_set(rb_cLVAR, "__member__", members);
    rb_iv_set(rb_cLVAR, "__type__", INT2NUM(NODE_LVAR));
    rb_define_singleton_method(rb_cLVAR, "members", node_s_members, 0);

    /* Document-method: vid
     * the name of the local variable to retrieve.
     */
    rb_define_method(rb_cLVAR, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));
    rb_define_method(rb_cLVAR, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::MASGN
   * Represents multiple assignment.
   */
  {
    VALUE rb_cMASGN = rb_define_class_under(rb_cNode, "MASGN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_MASGN] = rb_cMASGN;
    rb_iv_set(rb_cMASGN, "__member__", members);
    rb_iv_set(rb_cMASGN, "__type__", INT2NUM(NODE_MASGN));
    rb_define_singleton_method(rb_cMASGN, "members", node_s_members, 0);

    /* Document-method: args
     * TODO
     */
    rb_define_method(rb_cMASGN, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));

    /* Document-method: head
     * TODO
     */
    rb_define_method(rb_cMASGN, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: value
     * TODO
     */
    rb_define_method(rb_cMASGN, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));
  }

  /* Document-class: Node::MATCH
   * Represents a regular expression match in a conditional, e.g.:
   *   if /lit/ then
   *     ...
   *   end
   * 
   * This expression is equivalent to:
   *   if /lit/ =~ $_ then
   *     ...
   *   end
   * 
   * On ruby 1.8 and newer, this type of expression causes ruby to emit a
   * warning, unless script is running with -e.
   */
  {
    VALUE rb_cMATCH = rb_define_class_under(rb_cNode, "MATCH", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_MATCH] = rb_cMATCH;
    rb_iv_set(rb_cMATCH, "__member__", members);
    rb_iv_set(rb_cMATCH, "__type__", INT2NUM(NODE_MATCH));
    rb_define_singleton_method(rb_cMATCH, "members", node_s_members, 0);

    /* Document-method: lit
     * the regular expression to use in the condition.
     */
    rb_define_method(rb_cMATCH, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));

    /* Document-method: value
     * the value to compare against
     */
    rb_define_method(rb_cMATCH, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));
  }

  /* Document-class: Node::MATCH2
   * Represents a match in a conditional with a regular expression using
   * interpolation, e.g.:
   *   if /#{recv}/ then
   *     ...
   *   end
   * 
   * which is equivalent to:
   *   if /#{recv}/ =~ $_ then
   *     ...
   *   end
   * 
   * or a match with a regular expression on the left hand side and an
   * expression on the right hand side, e.g.:
   * 
   *   /recv/ =~ value
   */
  {
    VALUE rb_cMATCH2 = rb_define_class_under(rb_cNode, "MATCH2", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_MATCH2] = rb_cMATCH2;
    rb_iv_set(rb_cMATCH2, "__member__", members);
    rb_iv_set(rb_cMATCH2, "__type__", INT2NUM(NODE_MATCH2));
    rb_define_singleton_method(rb_cMATCH2, "members", node_s_members, 0);

    /* Document-method: value
     * the expression on the right hand side of the match operator, or
     * an expression returning $_ if there is nothing on the right hand
     * side
     */
    rb_define_method(rb_cMATCH2, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: recv
     * the regular expression on the left hand side of the match
     * operator
     */
    rb_define_method(rb_cMATCH2, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::MATCH3
   * Represents a regular expression match of the form:
   *   recv =~ /value/
   * 
   * where recv is an expression that returns an object and value is a
   * regular expression literal.
   */
  {
    VALUE rb_cMATCH3 = rb_define_class_under(rb_cNode, "MATCH3", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_MATCH3] = rb_cMATCH3;
    rb_iv_set(rb_cMATCH3, "__member__", members);
    rb_iv_set(rb_cMATCH3, "__type__", INT2NUM(NODE_MATCH3));
    rb_define_singleton_method(rb_cMATCH3, "members", node_s_members, 0);

    /* Document-method: value
     * the right hand side of the match
     */
    rb_define_method(rb_cMATCH3, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: recv
     * the left hand side of the match
     */
    rb_define_method(rb_cMATCH3, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::MEMO
   * A node used for temporary storage.
   * 
   * It is used by the rb_protect function so that the cont_protect
   * variable can be restored when the function returns.
   * 
   * It is also used in the Enumerable module and by autoload as a
   * temporary placeholder.
   * 
   * It should never be evaluated as an expression.
   * 
   * It holds up to three nodes or ruby objects, depending on how it is
   * used.
   */
  {
    VALUE rb_cMEMO = rb_define_class_under(rb_cNode, "MEMO", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_MEMO] = rb_cMEMO;
    rb_iv_set(rb_cMEMO, "__member__", members);
    rb_iv_set(rb_cMEMO, "__type__", INT2NUM(NODE_MEMO));
    rb_define_singleton_method(rb_cMEMO, "members", node_s_members, 0);
  }

  /* Document-class: Node::METHOD
   * A placeholder for a method entry in a class's method table.
   */
  {
    VALUE rb_cMETHOD = rb_define_class_under(rb_cNode, "METHOD", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_METHOD] = rb_cMETHOD;
    rb_iv_set(rb_cMETHOD, "__member__", members);
    rb_iv_set(rb_cMETHOD, "__type__", INT2NUM(NODE_METHOD));
    rb_define_singleton_method(rb_cMETHOD, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the method
     */
    rb_define_method(rb_cMETHOD, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: noex
     * the method's flags
     */
    rb_define_method(rb_cMETHOD, "noex", node_noex, 0);
    rb_ary_push(members, rb_str_new2("noex"));
  }

  /* Document-class: Node::MODULE
   * Represents a module definition, e.g.:
   *   module cpath
   *     body
   *   end
   * 
   * The module definition is evaluated in a new lexical scope.
   * 
   * The result of the expression is the last expression evaluated in the
   * body.
   */
  {
    VALUE rb_cMODULE = rb_define_class_under(rb_cNode, "MODULE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_MODULE] = rb_cMODULE;
    rb_iv_set(rb_cMODULE, "__member__", members);
    rb_iv_set(rb_cMODULE, "__type__", INT2NUM(NODE_MODULE));
    rb_define_singleton_method(rb_cMODULE, "members", node_s_members, 0);

    /* Document-method: cpath
     * the name of the module to define
     */
    rb_define_method(rb_cMODULE, "cpath", node_cpath, 0);
    rb_ary_push(members, rb_str_new2("cpath"));

    /* Document-method: body
     * the body of the module definition
     */
    rb_define_method(rb_cMODULE, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::NEWLINE
   * Represents the termination of a line.  This is used for calling
   * event hooks when a new line of code is reached.
   */
  {
    VALUE rb_cNEWLINE = rb_define_class_under(rb_cNode, "NEWLINE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_NEWLINE] = rb_cNEWLINE;
    rb_iv_set(rb_cNEWLINE, "__member__", members);
    rb_iv_set(rb_cNEWLINE, "__type__", INT2NUM(NODE_NEWLINE));
    rb_define_singleton_method(rb_cNEWLINE, "members", node_s_members, 0);

    /* Document-method: next
     * the expression on the next line
     */
    rb_define_method(rb_cNEWLINE, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::NEXT
   * Represents the 'next' keyword.
   * Causes control to be transferred to the end of the loop, causing the
   * next value in the sequence to be retrieved.
   */
  {
    VALUE rb_cNEXT = rb_define_class_under(rb_cNode, "NEXT", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_NEXT] = rb_cNEXT;
    rb_iv_set(rb_cNEXT, "__member__", members);
    rb_iv_set(rb_cNEXT, "__type__", INT2NUM(NODE_NEXT));
    rb_define_singleton_method(rb_cNEXT, "members", node_s_members, 0);
    rb_define_method(rb_cNEXT, "stts", node_stts, 0);
    rb_ary_push(members, rb_str_new2("stts"));
  }

  /* Document-class: Node::NIL
   * Represents the keyword 'nil'.
   */
  {
    VALUE rb_cNIL = rb_define_class_under(rb_cNode, "NIL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_NIL] = rb_cNIL;
    rb_iv_set(rb_cNIL, "__member__", members);
    rb_iv_set(rb_cNIL, "__type__", INT2NUM(NODE_NIL));
    rb_define_singleton_method(rb_cNIL, "members", node_s_members, 0);
  }

  /* Document-class: Node::NOT
   * Represents a logical negation.
   */
  {
    VALUE rb_cNOT = rb_define_class_under(rb_cNode, "NOT", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_NOT] = rb_cNOT;
    rb_iv_set(rb_cNOT, "__member__", members);
    rb_iv_set(rb_cNOT, "__type__", INT2NUM(NODE_NOT));
    rb_define_singleton_method(rb_cNOT, "members", node_s_members, 0);

    /* Document-method: body
     * the value to negate.
     */
    rb_define_method(rb_cNOT, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::NTH_REF
   * Represents the nth match data item, e.g. $1, $2, etc.
   */
  {
    VALUE rb_cNTH_REF = rb_define_class_under(rb_cNode, "NTH_REF", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_NTH_REF] = rb_cNTH_REF;
    rb_iv_set(rb_cNTH_REF, "__member__", members);
    rb_iv_set(rb_cNTH_REF, "__type__", INT2NUM(NODE_NTH_REF));
    rb_define_singleton_method(rb_cNTH_REF, "members", node_s_members, 0);

    /* Document-method: nth
     * the index of the match data item to retrieve
     */
    rb_define_method(rb_cNTH_REF, "nth", node_nth, 0);
    rb_ary_push(members, rb_str_new2("nth"));

    /* Document-method: cnt
     * the index into the local variable table where the match data is stored
     */
    rb_define_method(rb_cNTH_REF, "cnt", node_cnt, 0);
    rb_ary_push(members, rb_str_new2("cnt"));
  }

  /* Document-class: Node::OPT_N
   * Represents the top-level loop when the -n or -p options are used
   * with the interpreter.
   */
  {
    VALUE rb_cOPT_N = rb_define_class_under(rb_cNode, "OPT_N", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OPT_N] = rb_cOPT_N;
    rb_iv_set(rb_cOPT_N, "__member__", members);
    rb_iv_set(rb_cOPT_N, "__type__", INT2NUM(NODE_OPT_N));
    rb_define_singleton_method(rb_cOPT_N, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the loop
     */
    rb_define_method(rb_cOPT_N, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::OP_ASGN1
   * Represents bracket assignment of the form:
   *   recv[index] += value or 
   *   recv[index] ||= value or
   *   recv[index] &&= value.
   * 
   * The index is obtained from args.body.
   * 
   * The value is obtained from args.head.
   * 
   * In the case of ||=, mid will be 0.  The rhs will be equal to the
   * result of evaluating args.head if the lhs is false, otherwise the
   * rhs will be equal to lhs.
   * 
   * In the case of &&=, mid will be 1.  The rhs will be equal to the lhs
   * if lhs is false, otherwise the rhs will be equal to the result of
   * evaluating args.head.  In all other cases, mid will be the name of
   * the method to call to calculate value, such that the expression is
   * equivalent to:
   *   recv[args.body] = recv[args.body].mid(args.head)
   * 
   * In no case does ruby short-circuit the assignment.
   */
  {
    VALUE rb_cOP_ASGN1 = rb_define_class_under(rb_cNode, "OP_ASGN1", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OP_ASGN1] = rb_cOP_ASGN1;
    rb_iv_set(rb_cOP_ASGN1, "__member__", members);
    rb_iv_set(rb_cOP_ASGN1, "__type__", INT2NUM(NODE_OP_ASGN1));
    rb_define_singleton_method(rb_cOP_ASGN1, "members", node_s_members, 0);

    /* Document-method: args
     * the arguments to the assigment
     */
    rb_define_method(rb_cOP_ASGN1, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));

    /* Document-method: mid
     * 0, 1, or the name a method to call to calculate the value of the
     * rhs
     */
    rb_define_method(rb_cOP_ASGN1, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));

    /* Document-method: recv
     * the receiver of the assignment
     */
    rb_define_method(rb_cOP_ASGN1, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::OP_ASGN2
   * Represents attribute assignment of the form:
   *   recv.attr op value
   * 
   * where recv is the receiver of the attr method, attr is the attribute
   * to which to assign, op is an assignment operation (e.g. +=), and
   * value is the value to assign to the attribute.
   * 
   * The 'next' member of this class is also of type OP_ASGN2, though it
   * has different members than its parent.  This child node is
   * documented under OP_ASGN2_ARG.
   */
  {
    VALUE rb_cOP_ASGN2 = rb_define_class_under(rb_cNode, "OP_ASGN2", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OP_ASGN2] = rb_cOP_ASGN2;
    rb_iv_set(rb_cOP_ASGN2, "__member__", members);
    rb_iv_set(rb_cOP_ASGN2, "__type__", INT2NUM(NODE_OP_ASGN2));
    rb_define_singleton_method(rb_cOP_ASGN2, "members", node_s_members, 0);

    /* Document-method: value
     * the value to assign to the attribute
     */
    rb_define_method(rb_cOP_ASGN2, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: next
     * another node of type OP_ASGN2 which contains more information
     * about the assignment operation than can fit in this node alone
     */
    rb_define_method(rb_cOP_ASGN2, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));

    /* Document-method: recv
     * the receiver of the attribute
     */
    rb_define_method(rb_cOP_ASGN2, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::OP_ASGN2_ARG
   * Actually a node of type OP_ASGN2, this is a placeholder for
   * additional information about the assignment than can fit in a single
   * OP_ASGN2 node.
   */
  {
    VALUE rb_cOP_ASGN2_ARG = rb_define_class_under(rb_cNode, "OP_ASGN2_ARG", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OP_ASGN2_ARG] = rb_cOP_ASGN2_ARG;
    rb_iv_set(rb_cOP_ASGN2_ARG, "__member__", members);
    rb_iv_set(rb_cOP_ASGN2_ARG, "__type__", INT2NUM(NODE_OP_ASGN2_ARG));
    rb_define_singleton_method(rb_cOP_ASGN2_ARG, "members", node_s_members, 0);

    /* Document-method: vid
     * The method to call on the receiver to retrieve the attribute
     */
    rb_define_method(rb_cOP_ASGN2_ARG, "vid", node_vid, 0);
    rb_ary_push(members, rb_str_new2("vid"));

    /* Document-method: aid
     * The method to call on the receiver to set the attribute
     */
    rb_define_method(rb_cOP_ASGN2_ARG, "aid", node_aid, 0);
    rb_ary_push(members, rb_str_new2("aid"));

    /* Document-method: mid
     * The operation to apply to the attribute before setting it.  May
     * be 0 (false) to indicate "logical or" or 1 (nil) to indicate
     * "logical and".
     */
    rb_define_method(rb_cOP_ASGN2_ARG, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));
  }

  /* Document-class: Node::OP_ASGN_AND
   * Represents an expression of the form:
   *   recv &&= value
   * Ruby will evaluate the expression on the left hand side of the
   * assignment; if it is true, then it will assign the result of the
   * expression on the right hand side to the receiver on the left hand
   * side.
   */
  {
    VALUE rb_cOP_ASGN_AND = rb_define_class_under(rb_cNode, "OP_ASGN_AND", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OP_ASGN_AND] = rb_cOP_ASGN_AND;
    rb_iv_set(rb_cOP_ASGN_AND, "__member__", members);
    rb_iv_set(rb_cOP_ASGN_AND, "__type__", INT2NUM(NODE_OP_ASGN_AND));
    rb_define_singleton_method(rb_cOP_ASGN_AND, "members", node_s_members, 0);

    /* Document-method: value
     * an expression representing the assignment that should be
     * performed if the left hand side is true
     */
    rb_define_method(rb_cOP_ASGN_AND, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: recv
     * an expression representing the left hand side of the assignment
     */
    rb_define_method(rb_cOP_ASGN_AND, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::OP_ASGN_OR
   * Represents an expression of the form:
   *   recv ||= value
   * Ruby will evaluate the expression on the left hand side.  If it is
   * undefined, nil, or false, then ruby will evaluate the expression on
   * the right hand side and assign it to recv.  The expression will
   * short-circuit if recv is defined and true (non-false, non-nil).
   */
  {
    VALUE rb_cOP_ASGN_OR = rb_define_class_under(rb_cNode, "OP_ASGN_OR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OP_ASGN_OR] = rb_cOP_ASGN_OR;
    rb_iv_set(rb_cOP_ASGN_OR, "__member__", members);
    rb_iv_set(rb_cOP_ASGN_OR, "__type__", INT2NUM(NODE_OP_ASGN_OR));
    rb_define_singleton_method(rb_cOP_ASGN_OR, "members", node_s_members, 0);

    /* Document-method: aid
     * if this indicator is nonzero, ruby will check to see if the
     * provided expression is defined, otherwise it will assume that
     * the provided expression is defined.
     */
    rb_define_method(rb_cOP_ASGN_OR, "aid", node_aid, 0);
    rb_ary_push(members, rb_str_new2("aid"));

    /* Document-method: value
     * the right hand side of the assignment
     */
    rb_define_method(rb_cOP_ASGN_OR, "value", node_value, 0);
    rb_ary_push(members, rb_str_new2("value"));

    /* Document-method: recv
     * the receiver of the assignment
     */
    rb_define_method(rb_cOP_ASGN_OR, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::OR
   * Represents a logical 'or' of the form:
   *   first || second
   * The expression will short-circuit and yield the result of the left
   * hand side if it is true, else it will evaluate the right hand side
   * and use it as the result of the expression.
   */
  {
    VALUE rb_cOR = rb_define_class_under(rb_cNode, "OR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_OR] = rb_cOR;
    rb_iv_set(rb_cOR, "__member__", members);
    rb_iv_set(rb_cOR, "__type__", INT2NUM(NODE_OR));
    rb_define_singleton_method(rb_cOR, "members", node_s_members, 0);

    /* Document-method: first
     * the expression on the left hand side
     */
    rb_define_method(rb_cOR, "first", node_1st, 0);
    rb_ary_push(members, rb_str_new2("first"));

    /* Document-method: second
     * the expression on the right hand side
     */
    rb_define_method(rb_cOR, "second", node_2nd, 0);
    rb_ary_push(members, rb_str_new2("second"));
  }

  /* Document-class: Node::POSTEXE
   * Represents the END keyword, e.g.:
   *   END { ... }
   * 
   * Indicating that the enclosing ITER node is to be excecuted only
   * once, when the program terminates.
   */
  {
    VALUE rb_cPOSTEXE = rb_define_class_under(rb_cNode, "POSTEXE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_POSTEXE] = rb_cPOSTEXE;
    rb_iv_set(rb_cPOSTEXE, "__member__", members);
    rb_iv_set(rb_cPOSTEXE, "__type__", INT2NUM(NODE_POSTEXE));
    rb_define_singleton_method(rb_cPOSTEXE, "members", node_s_members, 0);
  }

  /* Document-class: Node::REDO
   * Represents the 'redo' keyword.  Causes control to be transferred to
   * the beginning of the loop.  The loop assignment is not repeated.
   */
  {
    VALUE rb_cREDO = rb_define_class_under(rb_cNode, "REDO", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_REDO] = rb_cREDO;
    rb_iv_set(rb_cREDO, "__member__", members);
    rb_iv_set(rb_cREDO, "__type__", INT2NUM(NODE_REDO));
    rb_define_singleton_method(rb_cREDO, "members", node_s_members, 0);
  }

  /* Document-class: Node::RESBODY
   * Represents the rescue portion of a rescue expression (see RESCUE for
   * examples).
   * 
   * If the head node of the rescue expresion raises an exception, the
   * resq node is evaluated.  The resq node is of type RESBDOY.
   * 
   * As it is evaluated, the type of the exception is tested against the
   * class(es) listed in the args node.  If there is a match, the body
   * node is evaluated, otherwise the head node is evaluated.  The head
   * node is either another RESBDOY node or false (0).
   */
  {
    VALUE rb_cRESBODY = rb_define_class_under(rb_cNode, "RESBODY", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_RESBODY] = rb_cRESBODY;
    rb_iv_set(rb_cRESBODY, "__member__", members);
    rb_iv_set(rb_cRESBODY, "__type__", INT2NUM(NODE_RESBODY));
    rb_define_singleton_method(rb_cRESBODY, "members", node_s_members, 0);

    /* Document-method: head
     * the next rescue
     */
    rb_define_method(rb_cRESBODY, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: args
     * the expression type to match against
     */
    rb_define_method(rb_cRESBODY, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));

    /* Document-method: body
     * the expresion to evaluate if the exception type matches
     */
    rb_define_method(rb_cRESBODY, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::RESCUE
   * Represents part of a rescue expression of the form:
   *   head rescue expr
   * 
   * or the longer form:
   *   begin
   *     head
   *   rescue exception
   *     expr
   *   end
   * 
   * or the form using the else keyword:
   *   begin
   *     head
   *   rescue exception
   *     expr
   *   else
   *     else_expr
   *   end
   * 
   * The head expression is first evaluated, and if an exception is
   * raised, evaluates the resq node.  If no exception is raised, the
   * else node is evaluated if it is not false (0).
   * 
   * The resq node will be a RESBDOY node, which will test the exception
   * to see if it matches one of the expected types.  If it does it will
   * handle the exception, otherwise it will allow the expression to pass
   * to the outer scope.
   */
  {
    VALUE rb_cRESCUE = rb_define_class_under(rb_cNode, "RESCUE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_RESCUE] = rb_cRESCUE;
    rb_iv_set(rb_cRESCUE, "__member__", members);
    rb_iv_set(rb_cRESCUE, "__type__", INT2NUM(NODE_RESCUE));
    rb_define_singleton_method(rb_cRESCUE, "members", node_s_members, 0);

    /* Document-method: head
     * the body of the block to evaluate
     */
    rb_define_method(rb_cRESCUE, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: else
     * the expression to be evaluated if no exception is raised
     */
    rb_define_method(rb_cRESCUE, "else", node_else, 0);
    rb_ary_push(members, rb_str_new2("else"));

    /* Document-method: resq
     * the expression to be evaluated if an exception is raised
     */
    rb_define_method(rb_cRESCUE, "resq", node_resq, 0);
    rb_ary_push(members, rb_str_new2("resq"));
  }

  /* Document-class: Node::RETRY
   * Represents the retry keyword.
   */
  {
    VALUE rb_cRETRY = rb_define_class_under(rb_cNode, "RETRY", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_RETRY] = rb_cRETRY;
    rb_iv_set(rb_cRETRY, "__member__", members);
    rb_iv_set(rb_cRETRY, "__type__", INT2NUM(NODE_RETRY));
    rb_define_singleton_method(rb_cRETRY, "members", node_s_members, 0);
  }

  /* Document-class: Node::RETURN
   * Represents the 'return' keyword.  Evaluation of this node results in
   * a return from the current method.  If no argument is supplied,
   * returns nil, otherwise returns the result of the supplied
   * expression.
   */
  {
    VALUE rb_cRETURN = rb_define_class_under(rb_cNode, "RETURN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_RETURN] = rb_cRETURN;
    rb_iv_set(rb_cRETURN, "__member__", members);
    rb_iv_set(rb_cRETURN, "__type__", INT2NUM(NODE_RETURN));
    rb_define_singleton_method(rb_cRETURN, "members", node_s_members, 0);

    /* Document-method: stts
     * an expression representing the value to return
     */
    rb_define_method(rb_cRETURN, "stts", node_stts, 0);
    rb_ary_push(members, rb_str_new2("stts"));
  }

  /* Document-class: Node::SCLASS
   * Represents the body of a singleton class definition, e.g.:
   *   class << recv
   *     body
   *   end
   * 
   * The class definition is evaluated in a new lexical scope.
   * 
   * The result of the expression is the last expression evaluated in the
   * body.
   */
  {
    VALUE rb_cSCLASS = rb_define_class_under(rb_cNode, "SCLASS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_SCLASS] = rb_cSCLASS;
    rb_iv_set(rb_cSCLASS, "__member__", members);
    rb_iv_set(rb_cSCLASS, "__type__", INT2NUM(NODE_SCLASS));
    rb_define_singleton_method(rb_cSCLASS, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the class definition
     */
    rb_define_method(rb_cSCLASS, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: recv
     * the object whose singleton class is to be modified
     */
    rb_define_method(rb_cSCLASS, "recv", node_recv, 0);
    rb_ary_push(members, rb_str_new2("recv"));
  }

  /* Document-class: Node::SCOPE
   * Represents a lexical scope.
   * 
   * A new scope is created when a method is invoked.  The scope node
   * holds information about local variables and arguments to the method.
   * The first two variables in the local variable table are the implicit
   * variables $_ and $~.
   * 
   * The next variables listed in the local variable table are the
   * arguments to the method.  More information about the arguments to
   * the method are stored in the ARGS node, which will either be the
   * first node in the scope or the first node in the BLOCK held by the
   * scope.
   */
  {
    VALUE rb_cSCOPE = rb_define_class_under(rb_cNode, "SCOPE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_SCOPE] = rb_cSCOPE;
    rb_iv_set(rb_cSCOPE, "__member__", members);
    rb_iv_set(rb_cSCOPE, "__type__", INT2NUM(NODE_SCOPE));
    rb_define_singleton_method(rb_cSCOPE, "members", node_s_members, 0);

    /* Document-method: tbl
     * the names of the local variables* next the first expression in
     * the scope
     */
    rb_define_method(rb_cSCOPE, "tbl", node_tbl, 0);
    rb_ary_push(members, rb_str_new2("tbl"));

    /* Document-method: rval
     * holds information about which class(es) to search for  constants
     * in this scope
     */
    rb_define_method(rb_cSCOPE, "rval", node_rval, 0);
    rb_ary_push(members, rb_str_new2("rval"));

    /* Document-method: next
     * the body of the lexical scope
     */
    rb_define_method(rb_cSCOPE, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::SELF
   * Represents the keyword 'self'.
   */
  {
    VALUE rb_cSELF = rb_define_class_under(rb_cNode, "SELF", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_SELF] = rb_cSELF;
    rb_iv_set(rb_cSELF, "__member__", members);
    rb_iv_set(rb_cSELF, "__type__", INT2NUM(NODE_SELF));
    rb_define_singleton_method(rb_cSELF, "members", node_s_members, 0);
  }

  /* Document-class: Node::SPLAT
   * Represents the splat (*) operation as an rvalue, e.g.:
   *   *head
   * If the argument is an array, returns self.
   * If the argument is nil, returns [nil].
   * If the argument is any other value, returns the result of calling #to_a on the
   * argument.
   */
  {
    VALUE rb_cSPLAT = rb_define_class_under(rb_cNode, "SPLAT", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_SPLAT] = rb_cSPLAT;
    rb_iv_set(rb_cSPLAT, "__member__", members);
    rb_iv_set(rb_cSPLAT, "__type__", INT2NUM(NODE_SPLAT));
    rb_define_singleton_method(rb_cSPLAT, "members", node_s_members, 0);

    /* Document-method: head
     * the argument to splat.
     */
    rb_define_method(rb_cSPLAT, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
  }

  /* Document-class: Node::STR
   * Represents a string object.  Duplicates the string stored in the
   * node.
   */
  {
    VALUE rb_cSTR = rb_define_class_under(rb_cNode, "STR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_STR] = rb_cSTR;
    rb_iv_set(rb_cSTR, "__member__", members);
    rb_iv_set(rb_cSTR, "__type__", INT2NUM(NODE_STR));
    rb_define_singleton_method(rb_cSTR, "members", node_s_members, 0);

    /* Document-method: lit
     * the string to be duplicated
     */
    rb_define_method(rb_cSTR, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));
  }

  /* Document-class: Node::SUPER
   * Represents the keyword 'super' when used with parens or with arguments.
   */
  {
    VALUE rb_cSUPER = rb_define_class_under(rb_cNode, "SUPER", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_SUPER] = rb_cSUPER;
    rb_iv_set(rb_cSUPER, "__member__", members);
    rb_iv_set(rb_cSUPER, "__type__", INT2NUM(NODE_SUPER));
    rb_define_singleton_method(rb_cSUPER, "members", node_s_members, 0);

    /* Document-method: args
     * the arguments to be passed to the base class
     */
    rb_define_method(rb_cSUPER, "args", node_args, 0);
    rb_ary_push(members, rb_str_new2("args"));
  }

  /* Document-class: Node::SVALUE
   * Represents the collection of multiple values of the right hand side
   * of an assignment into a single value, for use in single assignment,
   * e.g.:
   *   lhs = a, b, c
   * The argument to this node is an Array.
   * If it is length 0, returns nil.
   * If it is length 1, returns the first element in the array.
   * Otherwise, returns the array.
   */
  {
    VALUE rb_cSVALUE = rb_define_class_under(rb_cNode, "SVALUE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_SVALUE] = rb_cSVALUE;
    rb_iv_set(rb_cSVALUE, "__member__", members);
    rb_iv_set(rb_cSVALUE, "__type__", INT2NUM(NODE_SVALUE));
    rb_define_singleton_method(rb_cSVALUE, "members", node_s_members, 0);

    /* Document-method: head
     * an expression which returns an Array.
     */
    rb_define_method(rb_cSVALUE, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
  }

  /* Document-class: Node::TO_ARY
   * Represents a conversion from one object type to an array type.
   * Evaluation of this node converts its argument to an array by calling
   * \#to_ary on the argument.
   */
  {
    VALUE rb_cTO_ARY = rb_define_class_under(rb_cNode, "TO_ARY", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_TO_ARY] = rb_cTO_ARY;
    rb_iv_set(rb_cTO_ARY, "__member__", members);
    rb_iv_set(rb_cTO_ARY, "__type__", INT2NUM(NODE_TO_ARY));
    rb_define_singleton_method(rb_cTO_ARY, "members", node_s_members, 0);

    /* Document-method: head
     * the object to convert to an array
     */
    rb_define_method(rb_cTO_ARY, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));
  }

  /* Document-class: Node::TRUE
   * Represents the keyword 'true'.
   */
  {
    VALUE rb_cTRUE = rb_define_class_under(rb_cNode, "TRUE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_TRUE] = rb_cTRUE;
    rb_iv_set(rb_cTRUE, "__member__", members);
    rb_iv_set(rb_cTRUE, "__type__", INT2NUM(NODE_TRUE));
    rb_define_singleton_method(rb_cTRUE, "members", node_s_members, 0);
  }

  /* Document-class: Node::UNDEF
   * Represents an expression using the undef keyword, e.g.:
   *   undef :mid
   * 
   * This causes the method identified by mid in the current class to be
   * undefined.
   */
  {
    VALUE rb_cUNDEF = rb_define_class_under(rb_cNode, "UNDEF", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_UNDEF] = rb_cUNDEF;
    rb_iv_set(rb_cUNDEF, "__member__", members);
    rb_iv_set(rb_cUNDEF, "__type__", INT2NUM(NODE_UNDEF));
    rb_define_singleton_method(rb_cUNDEF, "members", node_s_members, 0);

    /* Document-method: body
     * an expression returning the id of the method to undefine
     */
    rb_define_method(rb_cUNDEF, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));
  }

  /* Document-class: Node::UNTIL
   * Represents a loop constructed with the 'until' keyword, e.g.:
   *   until cond do
   *     body
   *   end
   */
  {
    VALUE rb_cUNTIL = rb_define_class_under(rb_cNode, "UNTIL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_UNTIL] = rb_cUNTIL;
    rb_iv_set(rb_cUNTIL, "__member__", members);
    rb_iv_set(rb_cUNTIL, "__type__", INT2NUM(NODE_UNTIL));
    rb_define_singleton_method(rb_cUNTIL, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the loop
     */
    rb_define_method(rb_cUNTIL, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: cond
     * a condition to terminate the loop when it becomes true
     */
    rb_define_method(rb_cUNTIL, "cond", node_cond, 0);
    rb_ary_push(members, rb_str_new2("cond"));
    rb_define_method(rb_cUNTIL, "state", node_state, 0);
    rb_ary_push(members, rb_str_new2("state"));
  }

  /* Document-class: Node::VALIAS
   * Represents an alias expression of the form:
   *   alias 1st 2nd
   * where 2nd is the name of a variable and 1st is the name of its new
   * alias.
   */
  {
    VALUE rb_cVALIAS = rb_define_class_under(rb_cNode, "VALIAS", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_VALIAS] = rb_cVALIAS;
    rb_iv_set(rb_cVALIAS, "__member__", members);
    rb_iv_set(rb_cVALIAS, "__type__", INT2NUM(NODE_VALIAS));
    rb_define_singleton_method(rb_cVALIAS, "members", node_s_members, 0);
    rb_define_method(rb_cVALIAS, "first", node_1st, 0);
    rb_ary_push(members, rb_str_new2("first"));
    rb_define_method(rb_cVALIAS, "second", node_2nd, 0);
    rb_ary_push(members, rb_str_new2("second"));
  }

  /* Document-class: Node::VCALL
   * Represents a local variable or a method call without an explicit
   * receiver, to be determined at run-time.
   */
  {
    VALUE rb_cVCALL = rb_define_class_under(rb_cNode, "VCALL", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_VCALL] = rb_cVCALL;
    rb_iv_set(rb_cVCALL, "__member__", members);
    rb_iv_set(rb_cVCALL, "__type__", INT2NUM(NODE_VCALL));
    rb_define_singleton_method(rb_cVCALL, "members", node_s_members, 0);

    /* Document-method: mid
     * the name of the variable or method
     */
    rb_define_method(rb_cVCALL, "mid", node_mid, 0);
    rb_ary_push(members, rb_str_new2("mid"));
  }

  /* Document-class: Node::WHEN
   * Represents either a conditional in a case/end block:
   *   case value
   *   when head
   *     body
   *   next
   *   end
   * 
   * or the first element of a a case/end block with no conditional:
   *   case
   *   when head
   *     body
   *   next
   *   end
   * 
   * In the first case, value is evaluated and head is evaluated, and
   * they are compared using head === value.  If the result is true, then
   * body is evaluated and no more when conditions are tried, otherwise
   * the interpreter evaluates the next node.
   * 
   * In the second case, head is tested using rtest(value).  If the
   * result is true, then body is evaluated and no more when conditions
   * are tried, otherwise the interpreter evaluates the next node.
   * 
   * If the condition evaluates to false and there are no more when
   * conditions to try, the result of the expression is nil.
   * 
   * If the next expression to try is not a WHEN node, then it represents
   * an else element and is evaluated unconditionally.
   */
  {
    VALUE rb_cWHEN = rb_define_class_under(rb_cNode, "WHEN", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_WHEN] = rb_cWHEN;
    rb_iv_set(rb_cWHEN, "__member__", members);
    rb_iv_set(rb_cWHEN, "__type__", INT2NUM(NODE_WHEN));
    rb_define_singleton_method(rb_cWHEN, "members", node_s_members, 0);

    /* Document-method: head
     * a value to compare against, or a condition to be tested
     */
    rb_define_method(rb_cWHEN, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: body
     * an expression to evaluate if the condition evaluates to true
     */
    rb_define_method(rb_cWHEN, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: next
     * the next expression to be evaluated if the condition is false
     */
    rb_define_method(rb_cWHEN, "next", node_next, 0);
    rb_ary_push(members, rb_str_new2("next"));
  }

  /* Document-class: Node::WHILE
   * Represents a loop constructed with the 'while' keyword, e.g.:
   *   while cond do
   *     body
   *   end
   */
  {
    VALUE rb_cWHILE = rb_define_class_under(rb_cNode, "WHILE", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_WHILE] = rb_cWHILE;
    rb_iv_set(rb_cWHILE, "__member__", members);
    rb_iv_set(rb_cWHILE, "__type__", INT2NUM(NODE_WHILE));
    rb_define_singleton_method(rb_cWHILE, "members", node_s_members, 0);

    /* Document-method: body
     * the body of the loop
     */
    rb_define_method(rb_cWHILE, "body", node_body, 0);
    rb_ary_push(members, rb_str_new2("body"));

    /* Document-method: cond
     * a condition to terminate the loop when it becomes false
     */
    rb_define_method(rb_cWHILE, "cond", node_cond, 0);
    rb_ary_push(members, rb_str_new2("cond"));
    rb_define_method(rb_cWHILE, "state", node_state, 0);
    rb_ary_push(members, rb_str_new2("state"));
  }

  /* Document-class: Node::XSTR
   * Represents a string object inside backticks, e.g.:
   *   `lit`
   * The given string is executed in a subshell and the output from its
   * stdout stored in a string.
   */
  {
    VALUE rb_cXSTR = rb_define_class_under(rb_cNode, "XSTR", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_XSTR] = rb_cXSTR;
    rb_iv_set(rb_cXSTR, "__member__", members);
    rb_iv_set(rb_cXSTR, "__type__", INT2NUM(NODE_XSTR));
    rb_define_singleton_method(rb_cXSTR, "members", node_s_members, 0);

    /* Document-method: lit
     * the string to be executed
     */
    rb_define_method(rb_cXSTR, "lit", node_lit, 0);
    rb_ary_push(members, rb_str_new2("lit"));
  }

  /* Document-class: Node::YIELD
   * Represents the 'yield' keyword.  When evaluated, causes control of
   * the program to be transferred to the block passed by the calling
   * scope.
   */
  {
    VALUE rb_cYIELD = rb_define_class_under(rb_cNode, "YIELD", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_YIELD] = rb_cYIELD;
    rb_iv_set(rb_cYIELD, "__member__", members);
    rb_iv_set(rb_cYIELD, "__type__", INT2NUM(NODE_YIELD));
    rb_define_singleton_method(rb_cYIELD, "members", node_s_members, 0);

    /* Document-method: head
     * the value to yield
     */
    rb_define_method(rb_cYIELD, "head", node_head, 0);
    rb_ary_push(members, rb_str_new2("head"));

    /* Document-method: state
     * if nonzero, splats the value before yielding
     */
    rb_define_method(rb_cYIELD, "state", node_state, 0);
    rb_ary_push(members, rb_str_new2("state"));
  }

  /* Document-class: Node::ZARRAY
   * Represents an array of zero elements.  Evalation of this node
   * creates a new array of length zero.
   */
  {
    VALUE rb_cZARRAY = rb_define_class_under(rb_cNode, "ZARRAY", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ZARRAY] = rb_cZARRAY;
    rb_iv_set(rb_cZARRAY, "__member__", members);
    rb_iv_set(rb_cZARRAY, "__type__", INT2NUM(NODE_ZARRAY));
    rb_define_singleton_method(rb_cZARRAY, "members", node_s_members, 0);
  }

  /* Document-class: Node::ZSUPER
   * Represents the keyword 'super' when used without parens nor
   * arguments.  The arguments to the base class method are obtained from
   * the arguments passed to the current method, which are store in the
   * current frame.
   * 
   * Can also be a placeholder for a method when its implementation is
   * deferred to the base class.
   */
  {
    VALUE rb_cZSUPER = rb_define_class_under(rb_cNode, "ZSUPER", rb_cNode);
    members = rb_ary_new();
    rb_cNodeSubclass[NODE_ZSUPER] = rb_cZSUPER;
    rb_iv_set(rb_cZSUPER, "__member__", members);
    rb_iv_set(rb_cZSUPER, "__type__", INT2NUM(NODE_ZSUPER));
    rb_define_singleton_method(rb_cZSUPER, "members", node_s_members, 0);
  }
}


