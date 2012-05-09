#include "node_type_descrip.h"
#include "version.h"

Node_Type_Descrip node_type_descrips_unsorted[] = {
  { NODE_ALIAS, NEN_2ND, NEN_1ST, NEN_NONE, "ALIAS" },
#ifdef HAVE_NODE_ALLOCA
  { NODE_ALLOCA, NEN_CFNC, NEN_CNT, NEN_VALUE, "ALLOCA" },
#endif
  { NODE_AND, NEN_2ND, NEN_1ST, NEN_NONE, "AND" },
  { NODE_ARGS, NEN_OPT, NEN_CNT, NEN_REST, "ARGS" },
  { NODE_ARGSCAT, NEN_BODY, NEN_HEAD, NEN_NONE, "ARGSCAT" },
  { NODE_ARGSPUSH, NEN_BODY, NEN_HEAD, NEN_NONE, "ARGSPUSH" },
  { NODE_ARRAY, NEN_NEXT, NEN_HEAD, NEN_ALEN, "ARRAY" },
  { NODE_ATTRASGN, NEN_ARGS, NEN_RECV, NEN_MID, "ATTRASGN" },
  { NODE_ATTRSET, NEN_VID, NEN_NONE, NEN_NONE, "ATTRSET" },
  { NODE_BACK_REF, NEN_NTH, NEN_CNT, NEN_NONE, "BACK_REF" },
  { NODE_BEGIN, NEN_BODY, NEN_NONE, NEN_NONE, "BEGIN" },
  { NODE_BLOCK, NEN_NEXT, NEN_HEAD, NEN_NONE, "BLOCK" },
  { NODE_BLOCK_ARG, NEN_CNT, NEN_NONE, NEN_NONE, "BLOCK_ARG" },
  { NODE_BLOCK_PASS, NEN_BODY, NEN_ITER, NEN_NONE, "BLOCK_PASS" },
  { NODE_BMETHOD, NEN_CVAL, NEN_NONE, NEN_NONE, "BMETHOD" },
  { NODE_BREAK, NEN_STTS, NEN_NONE, NEN_NONE, "BREAK" },
  { NODE_CALL, NEN_ARGS, NEN_RECV, NEN_MID, "CALL" },
  { NODE_CASE, NEN_BODY, NEN_NEXT, NEN_HEAD, "CASE" },
  { NODE_CDECL, NEN_VID, NEN_VALUE, NEN_NONE, "CDECL" },
  { NODE_CFUNC, NEN_ARGC, NEN_CFNC, NEN_NONE, "CFUNC" },
  { NODE_CLASS, NEN_BODY, NEN_SUPER, NEN_CPATH, "CLASS" },
  { NODE_COLON2, NEN_HEAD, NEN_MID, NEN_NONE, "COLON2" },
  { NODE_COLON3, NEN_MID, NEN_NONE, NEN_NONE, "COLON3" },
  { NODE_CONST, NEN_VID, NEN_NONE, NEN_NONE, "CONST" },
  { NODE_CVAR, NEN_VID, NEN_NONE, NEN_NONE, "CVAR" },
  { NODE_CVASGN, NEN_VID, NEN_VALUE, NEN_NONE, "CVASGN" },
  { NODE_CVDECL, NEN_VID, NEN_VALUE, NEN_NONE, "CVDECL" },
  { NODE_DASGN, NEN_VID, NEN_VALUE, NEN_NONE, "DASGN" },
  { NODE_DASGN_CURR, NEN_VID, NEN_VALUE, NEN_NONE, "DASGN_CURR" },
  { NODE_DEFINED, NEN_HEAD, NEN_NONE, NEN_NONE, "DEFINED" },
  { NODE_DEFN, NEN_NOEX, NEN_DEFN, NEN_MID, "DEFN" },
  { NODE_DEFS, NEN_DEFN, NEN_RECV, NEN_MID, "DEFS" },
  { NODE_DOT2, NEN_STATE, NEN_END, NEN_BEG, "DOT2" },
  { NODE_DOT3, NEN_STATE, NEN_END, NEN_BEG, "DOT3" },
  { NODE_DREGX, NEN_NEXT, NEN_CFLAG, NEN_LIT, "DREGX" },
  { NODE_DREGX_ONCE, NEN_NEXT, NEN_CFLAG, NEN_LIT, "DREGX_ONCE" },
  { NODE_DSTR, NEN_NEXT, NEN_LIT, NEN_NONE, "DSTR" },
  { NODE_DSYM, NEN_NEXT, NEN_LIT, NEN_NONE, "DSYM" },
  { NODE_DVAR, NEN_VID, NEN_NONE, NEN_NONE, "DVAR" },
  { NODE_DXSTR, NEN_NEXT, NEN_LIT, NEN_NONE, "DXSTR" },
  { NODE_ENSURE, NEN_HEAD, NEN_ENSR, NEN_NONE, "ENSURE" },
  { NODE_EVSTR, NEN_BODY, NEN_NONE, NEN_NONE, "EVSTR" },
  { NODE_FALSE, NEN_NONE, NEN_NONE, NEN_NONE, "FALSE" },
  { NODE_FCALL, NEN_ARGS, NEN_MID, NEN_NONE, "FCALL" },
  { NODE_FLIP2, NEN_CNT, NEN_END, NEN_BEG, "FLIP2" },
  { NODE_FLIP3, NEN_CNT, NEN_END, NEN_BEG, "FLIP3" },
  { NODE_FOR, NEN_BODY, NEN_ITER, NEN_VAR, "FOR" },
  { NODE_GASGN, NEN_ENTRY, NEN_VID, NEN_VALUE, "GASGN" },
  { NODE_GVAR, NEN_ENTRY, NEN_VID, NEN_NONE, "GVAR" },
  { NODE_HASH, NEN_HEAD, NEN_NONE, NEN_NONE, "HASH" },
  { NODE_IASGN, NEN_VID, NEN_VALUE, NEN_NONE, "IASGN" },
  { NODE_IF, NEN_BODY, NEN_ELSE, NEN_COND, "IF" },
  { NODE_IFUNC, NEN_TVAL, NEN_CFNC, NEN_STATE, "IFUNC" },
  { NODE_ITER, NEN_BODY, NEN_ITER, NEN_VAR, "ITER" },
  { NODE_IVAR, NEN_VID, NEN_NONE, NEN_NONE, "IVAR" },
  { NODE_LASGN, NEN_VID, NEN_CNT, NEN_VALUE, "LASGN" },
  { NODE_LIT, NEN_LIT, NEN_NONE, NEN_NONE, "LIT" },
  { NODE_LVAR, NEN_VID, NEN_CNT, NEN_NONE, "LVAR" },
  { NODE_MASGN, NEN_ARGS, NEN_HEAD, NEN_VALUE, "MASGN" },
  { NODE_MATCH, NEN_LIT, NEN_VALUE, NEN_NONE, "MATCH" },
  { NODE_MATCH2, NEN_RECV, NEN_VALUE, NEN_NONE, "MATCH2" },
  { NODE_MATCH3, NEN_RECV, NEN_VALUE, NEN_NONE, "MATCH3" },
  { NODE_MEMO, NEN_NONE, NEN_NONE, NEN_NONE, "MEMO" },
  { NODE_MODULE, NEN_BODY, NEN_CPATH, NEN_NONE, "MODULE" },
  { NODE_NEXT, NEN_STTS, NEN_NONE, NEN_NONE, "NEXT" },
  { NODE_NIL, NEN_NONE, NEN_NONE, NEN_NONE, "NIL" },
  { NODE_NTH_REF, NEN_NTH, NEN_CNT, NEN_NONE, "NTH_REF" },
  { NODE_OPT_N, NEN_BODY, NEN_NONE, NEN_NONE, "OPT_N" },
  { NODE_OP_ASGN1, NEN_ARGS, NEN_RECV, NEN_MID, "OP_ASGN1" },
  { NODE_OP_ASGN2, NEN_NEXT, NEN_RECV, NEN_VALUE, "OP_ASGN2" },
  { NODE_OP_ASGN2_ARG, NEN_AID, NEN_MID, NEN_VID, "OP_ASGN2_ARG" },
  { NODE_OP_ASGN_AND, NEN_RECV, NEN_VALUE, NEN_NONE, "OP_ASGN_AND" },
  { NODE_OP_ASGN_OR, NEN_AID, NEN_RECV, NEN_VALUE, "OP_ASGN_OR" },
  { NODE_OR, NEN_2ND, NEN_1ST, NEN_NONE, "OR" },
  { NODE_POSTEXE, NEN_NONE, NEN_NONE, NEN_NONE, "POSTEXE" },
  { NODE_REDO, NEN_NONE, NEN_NONE, NEN_NONE, "REDO" },
  { NODE_RESBODY, NEN_BODY, NEN_ARGS, NEN_HEAD, "RESBODY" },
  { NODE_RESCUE, NEN_HEAD, NEN_ELSE, NEN_RESQ, "RESCUE" },
  { NODE_RETRY, NEN_NONE, NEN_NONE, NEN_NONE, "RETRY" },
  { NODE_RETURN, NEN_STTS, NEN_NONE, NEN_NONE, "RETURN" },
  { NODE_SCLASS, NEN_BODY, NEN_RECV, NEN_NONE, "SCLASS" },
  { NODE_SCOPE, NEN_NEXT, NEN_TBL, NEN_RVAL, "SCOPE" },
  { NODE_SELF, NEN_NONE, NEN_NONE, NEN_NONE, "SELF" },
  { NODE_SPLAT, NEN_HEAD, NEN_NONE, NEN_NONE, "SPLAT" },
  { NODE_STR, NEN_LIT, NEN_NONE, NEN_NONE, "STR" },
  { NODE_SUPER, NEN_ARGS, NEN_NONE, NEN_NONE, "SUPER" },
  { NODE_TO_ARY, NEN_HEAD, NEN_NONE, NEN_NONE, "TO_ARY" },
  { NODE_TRUE, NEN_NONE, NEN_NONE, NEN_NONE, "TRUE" },
  { NODE_UNDEF, NEN_BODY, NEN_NONE, NEN_NONE, "UNDEF" },
  { NODE_UNTIL, NEN_BODY, NEN_STATE, NEN_COND, "UNTIL" },
  { NODE_VALIAS, NEN_2ND, NEN_1ST, NEN_NONE, "VALIAS" },
  { NODE_VCALL, NEN_MID, NEN_NONE, NEN_NONE, "VCALL" },
  { NODE_WHEN, NEN_BODY, NEN_NEXT, NEN_HEAD, "WHEN" },
  { NODE_WHILE, NEN_BODY, NEN_STATE, NEN_COND, "WHILE" },
  { NODE_XSTR, NEN_LIT, NEN_NONE, NEN_NONE, "XSTR" },
  { NODE_YIELD, NEN_HEAD, NEN_STATE, NEN_NONE, "YIELD" },
  { NODE_ZARRAY, NEN_NONE, NEN_NONE, NEN_NONE, "ZARRAY" },
  { NODE_ZSUPER, NEN_NONE, NEN_NONE, NEN_NONE, "ZSUPER" },
  { NODE_LAST, NEN_NONE, NEN_NONE, NEN_NONE, "LAST" },
};

static Node_Type_Descrip * node_type_descrips[NUM_NODE_TYPE_DESCRIPS];
static int node_type_descrips_initialized = 0;

static void init_node_type_descrips()
{
  if(!node_type_descrips_initialized)
  {
    Node_Type_Descrip * descrip;
    memset(node_type_descrips, 0, sizeof(node_type_descrips));
    for(descrip = node_type_descrips_unsorted;
        descrip->nt != NODE_LAST;
        ++descrip)
    {
      if(node_type_descrips[descrip->nt])
      {
        rb_raise(rb_eRuntimeError, "duplicate entry for node type %d (%s is also %s)\n", descrip->nt, descrip->name, node_type_descrips[descrip->nt]->name);
      }
      else
      {
        node_type_descrips[descrip->nt] = descrip;
      }
    }
    node_type_descrips_initialized = 1;
  }
}

/* Given a node, find out the types of the three elements it contains */
Node_Type_Descrip const * node_type_descrip(enum node_type nt)
{
  init_node_type_descrips();
  if(node_type_descrips[nt])
  {
    return node_type_descrips[nt];
  }
  else
  {
    rb_raise(rb_eArgError, "Unknown node type %d", nt);
  }
}

