#ifndef nodeinfo_h
#define nodeinfo_h

#include "ruby_internal_node.h"

#include "ruby.h"

enum Node_Elem_Name
{
  NEN_NONE,
  NEN_1ST,
  NEN_2ND,
  NEN_AID,
  NEN_ALEN,
  NEN_ARGC,
  NEN_ARGS,
  NEN_BEG,
  NEN_BODY,
  NEN_CFLAG,
  NEN_CFNC,
  NEN_CLSS,
  NEN_CNAME,
  NEN_CNT,
  NEN_COND,
  NEN_CVAL,
  NEN_DEFN,
  NEN_ELSE,
  NEN_END,
  NEN_ENSR,
  NEN_ENTRY,
  NEN_FRML,
  NEN_HEAD,
  NEN_IBDY,
  NEN_ITER,
  NEN_LIT,
  NEN_MID,
  NEN_MODL,
  NEN_NEW,
  NEN_NEXT,
  NEN_NOEX,
  NEN_NTH,
  NEN_OLD,
  NEN_OPT,
  NEN_ORIG,
  NEN_RECV,
  NEN_RESQ,
  NEN_REST,
  NEN_RVAL,
  NEN_STATE,
  NEN_STTS,
  NEN_SUPER,
  NEN_TAG,
  NEN_TBL,
  NEN_TVAL,
  NEN_VALUE,
  NEN_VAR,
  NEN_VID,
};

VALUE dump_node_elem(enum Node_Elem_Name nen, NODE * n, VALUE node_hash);
void load_node_elem(enum Node_Elem_Name nen, VALUE v, NODE * n, VALUE node_hash, VALUE id_hash);
VALUE node_s_members(VALUE klass);
void define_node_subclass_methods();

#define NODE_OP_ASGN2_ARG NODE_LAST + 1
#define NUM_NODE_TYPE_DESCRIPS NODE_OP_ASGN2_ARG + 1

#endif

