#include "node_type_descrip.h"
#include "version.h"

static Node_Type_Descrip node_type_descrips[] = {
  { NODE_METHOD, NEN_NOEX, NEN_BODY, NEN_CNT },
  { NODE_BLOCK, NEN_NEXT, NEN_HEAD, NEN_NONE },
  { NODE_POSTEXE, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_BEGIN, NEN_BODY, NEN_NONE, NEN_NONE },
  { NODE_MATCH, NEN_LIT, NEN_NONE, NEN_NONE },
  { NODE_MATCH2, NEN_RECV, NEN_VALUE, NEN_NONE },
  { NODE_MATCH3, NEN_RECV, NEN_VALUE, NEN_NONE },
  { NODE_OPT_N, NEN_BODY, NEN_NONE, NEN_NONE },
  { NODE_SELF, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_NIL, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_TRUE, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_FALSE, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_IF, NEN_BODY, NEN_ELSE, NEN_COND },
  { NODE_WHEN, NEN_HEAD, NEN_BODY, NEN_NEXT },
  { NODE_CASE, NEN_HEAD, NEN_BODY, NEN_NEXT },
  { NODE_WHILE, NEN_COND, NEN_BODY, NEN_NONE },
  { NODE_UNTIL, NEN_STATE, NEN_COND, NEN_BODY },
  { NODE_BLOCK_PASS, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_ITER, NEN_VAR, NEN_BODY, NEN_ITER },
  { NODE_FOR, NEN_VAR, NEN_BODY, NEN_ITER },
  { NODE_BREAK, NEN_STTS, NEN_NONE, NEN_NONE },
  { NODE_NEXT, NEN_STTS, NEN_NONE, NEN_NONE },
  { NODE_REDO, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_RETRY, NEN_NONE, NEN_NONE, NEN_NONE },
#if RUBY_VERSION_CODE < 180
  { NODE_RESTARGS, NEN_HEAD, NEN_NONE, NEN_NONE },
#if RUBY_VERSION_CODE >= 170
  { NODE_RESTARY, NEN_HEAD, NEN_NONE, NEN_NONE },
  { NODE_REXPAND, NEN_HEAD, NEN_NONE, NEN_NONE },
#endif
#endif
  { NODE_YIELD, NEN_STTS, NEN_NONE, NEN_NONE },
  { NODE_RESCUE, NEN_HEAD, NEN_RESQ, NEN_ELSE },
  { NODE_RESBODY, NEN_HEAD, NEN_RESQ, NEN_ENSR },
  { NODE_ENSURE, NEN_HEAD, NEN_ENSR, NEN_NONE },
  { NODE_AND, NEN_1ST, NEN_2ND, NEN_NONE },
  { NODE_OR, NEN_1ST, NEN_2ND, NEN_NONE },
  { NODE_NOT, NEN_BODY, NEN_NONE, NEN_NONE },
  { NODE_DOT2, NEN_BEG, NEN_END, NEN_STATE },
  { NODE_DOT3, NEN_BEG, NEN_END, NEN_STATE },
  { NODE_FLIP2, NEN_CNT, NEN_BEG, NEN_END },
  { NODE_FLIP3, NEN_CNT, NEN_BEG, NEN_END },
  { NODE_RETURN, NEN_STTS, NEN_NONE, NEN_NONE },
  { NODE_ARGS, NEN_CNT, NEN_REST, NEN_OPT },
  { NODE_ARGSCAT, NEN_HEAD, NEN_BODY, NEN_NONE },
  { NODE_ARGSPUSH, NEN_HEAD, NEN_BODY, NEN_NONE },
  { NODE_CALL, NEN_RECV, NEN_ARGS, NEN_MID },
  { NODE_FCALL, NEN_ARGS, NEN_MID, NEN_NONE },
  { NODE_VCALL, NEN_MID, NEN_NONE, NEN_NONE },
  { NODE_SUPER, NEN_ARGS, NEN_NONE, NEN_NONE },
  { NODE_ZSUPER, NEN_ARGS, NEN_NONE, NEN_NONE },
  { NODE_SCOPE, NEN_RVAL, NEN_TBL, NEN_NEXT },
  { NODE_OP_ASGN1, NEN_RECV, NEN_ARGS, NEN_MID },
  { NODE_OP_ASGN2, NEN_VID, NEN_RECV, NEN_VALUE },
  { NODE_OP_ASGN_AND, NEN_HEAD, NEN_VALUE, NEN_NONE },
  { NODE_OP_ASGN_OR, NEN_HEAD, NEN_VALUE, NEN_NONE },
  { NODE_MASGN, NEN_VALUE, NEN_HEAD, NEN_ARGS },
  { NODE_LASGN, NEN_VID, NEN_VALUE, NEN_CNT },
  { NODE_DASGN, NEN_VALUE, NEN_VID, NEN_NONE },
  { NODE_DASGN_CURR, NEN_VALUE, NEN_VID, NEN_NONE },
  { NODE_GASGN, NEN_VID, NEN_VALUE, NEN_ENTRY },
  { NODE_IASGN, NEN_VID, NEN_VALUE, NEN_NONE },
  { NODE_CDECL, NEN_VID, NEN_VALUE, NEN_NONE },
  { NODE_CVDECL, NEN_VALUE, NEN_VID, NEN_NONE },
  { NODE_CVASGN, NEN_VALUE, NEN_VID, NEN_NONE },
  { NODE_LVAR, NEN_CNT, NEN_VID, NEN_NONE },
  { NODE_DVAR, NEN_VID, NEN_NONE, NEN_NONE },
  { NODE_GVAR, NEN_VID, NEN_ENTRY, NEN_NONE },
  { NODE_IVAR, NEN_VID, NEN_NONE, NEN_NONE },
  { NODE_CONST, NEN_VID, NEN_NONE, NEN_NONE },
  { NODE_CVAR, NEN_VID, NEN_NONE, NEN_NONE },
  { NODE_BLOCK_ARG, NEN_CNT, NEN_NONE, NEN_NONE },
  { NODE_COLON2, NEN_HEAD, NEN_MID, NEN_NONE },
  { NODE_COLON3, NEN_MID, NEN_NONE, NEN_NONE },
  { NODE_NTH_REF, NEN_NTH, NEN_NONE, NEN_NONE },
  { NODE_BACK_REF, NEN_NTH, NEN_NONE, NEN_NONE },
  { NODE_HASH, NEN_HEAD, NEN_NONE, NEN_NONE },
  { NODE_ZARRAY, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_ARRAY, NEN_ALEN, NEN_HEAD, NEN_NEXT },
  { NODE_STR, NEN_LIT, NEN_NONE, NEN_NONE },
  { NODE_NEXT, NEN_LIT, NEN_CFLAG, NEN_NONE },
#if RUBY_VERSION_CODE >= 170 && RUBY_VERSION_CODE < 180
  { NODE_REGX, NEN_CFLAG, NEN_NONE, NEN_NONE },
#endif
  { NODE_DREGX_ONCE, NEN_LIT, NEN_NONE, NEN_NONE },
  { NODE_DXSTR, NEN_NONE, NEN_NONE, NEN_NONE },
  { NODE_XSTR, NEN_LIT, NEN_NONE, NEN_NONE },
  { NODE_LIT, NEN_LIT, NEN_NONE, NEN_NONE },
  { NODE_ATTRSET, NEN_VID, NEN_NONE, NEN_NONE },
  { NODE_DEFN, NEN_DEFN, NEN_MID, NEN_NOEX },
  { NODE_DEFS, NEN_DEFN, NEN_RECV, NEN_MID },
  { NODE_UNDEF, NEN_MID, NEN_NONE, NEN_NONE },
  { NODE_ALIAS, NEN_NEW, NEN_OLD, NEN_NONE },
  { NODE_VALIAS, NEN_NEW, NEN_OLD, NEN_NONE },
#if RUBY_VERSION_CODE < 180
  { NODE_CLASS, NEN_SUPER, NEN_CNAME, NEN_BODY },
  { NODE_MODULE, NEN_CNAME, NEN_BODY, NEN_NONE },
#else
  { NODE_CLASS, NEN_SUPER, NEN_CPATH, NEN_BODY },
  { NODE_MODULE, NEN_CPATH, NEN_BODY, NEN_NONE },
#endif
  { NODE_SCLASS, NEN_RECV, NEN_BODY, NEN_NONE },
  { NODE_DEFINED, NEN_HEAD, NEN_NONE, NEN_NONE },
  { NODE_NEWLINE, NEN_NTH, NEN_NEXT, NEN_NONE },
  { NODE_IFUNC, NEN_CFNC, NEN_TVAL, NEN_ARGC }, 
  { NODE_CFUNC, NEN_CFNC, NEN_TVAL, NEN_ARGC },
  { NODE_LAST, NEN_NONE, NEN_NONE, NEN_NONE },
};

/* Given a node, find out the types of the three elements it contains */
Node_Type_Descrip const * node_type_descrip(enum node_type nt)
{
  Node_Type_Descrip *descrip;
  for(descrip = node_type_descrips; descrip->nt != NODE_LAST; ++descrip)
  {
    if(descrip->nt == nt)
    {
      return descrip;
    }
  }
  rb_raise(rb_eArgError, "Unknown node type %d", nt);
}

