#include "node_type_descrip.h"
#include "version.h"

static Node_Type_Descrip node_type_descrips[] = {
  { NODE_METHOD      , NEN_NOEX  , NEN_BODY  , NEN_CNT   , "METHOD"      }, 
  { NODE_BLOCK       , NEN_NEXT  , NEN_HEAD  , NEN_NONE  , "BLOCK"       }, 
  { NODE_POSTEXE     , NEN_NONE  , NEN_NONE  , NEN_NONE  , "POSTEXE"     }, 
  { NODE_BEGIN       , NEN_BODY  , NEN_NONE  , NEN_NONE  , "BEGIN"       }, 
  { NODE_MATCH       , NEN_HEAD  , NEN_NONE  , NEN_NONE  , "MATCH"       }, 
  { NODE_MATCH2      , NEN_RECV  , NEN_VALUE , NEN_NONE  , "MATCH2"      }, 
  { NODE_MATCH3      , NEN_RECV  , NEN_VALUE , NEN_NONE  , "MATCH3"      }, 
  { NODE_OPT_N       , NEN_BODY  , NEN_NONE  , NEN_NONE  , "OPT_N"       }, 
  { NODE_SELF        , NEN_NONE  , NEN_NONE  , NEN_NONE  , "SELF"        }, 
  { NODE_NIL         , NEN_NONE  , NEN_NONE  , NEN_NONE  , "NILNODE"     }, 
  { NODE_TRUE        , NEN_NONE  , NEN_NONE  , NEN_NONE  , "TRUENODE"    }, 
  { NODE_FALSE       , NEN_NONE  , NEN_NONE  , NEN_NONE  , "FALSENODE"   }, 
  { NODE_IF          , NEN_BODY  , NEN_ELSE  , NEN_COND  , "IF"          }, 
  { NODE_WHEN        , NEN_HEAD  , NEN_BODY  , NEN_NEXT  , "WHEN"        }, 
  { NODE_CASE        , NEN_HEAD  , NEN_BODY  , NEN_NEXT  , "CASE"        }, 
  { NODE_WHILE       , NEN_COND  , NEN_BODY  , NEN_NONE  , "WHILE"       }, 
  { NODE_UNTIL       , NEN_STATE , NEN_COND  , NEN_BODY  , "UNTIL"       }, 
  { NODE_BLOCK_PASS  , NEN_NONE  , NEN_NONE  , NEN_NONE  , "BLOCK_PASS"  }, 
  { NODE_ITER        , NEN_VAR   , NEN_BODY  , NEN_ITER  , "ITER"        }, 
  { NODE_FOR         , NEN_VAR   , NEN_BODY  , NEN_ITER  , "FOR"         }, 
  { NODE_BREAK       , NEN_STTS  , NEN_NONE  , NEN_NONE  , "BREAK"       }, 
  { NODE_NEXT        , NEN_STTS  , NEN_NONE  , NEN_NONE  , "NEXT"        }, 
  { NODE_REDO        , NEN_NONE  , NEN_NONE  , NEN_NONE  , "REDO"        }, 
  { NODE_RETRY       , NEN_NONE  , NEN_NONE  , NEN_NONE  , "RETRY"       }, 
#if RUBY_VERSION_CODE < 180
  { NODE_RESTARGS    , NEN_HEAD  , NEN_NONE  , NEN_NONE  , "RESTARGS"    }, 
#if RUBY_VERSION_CODE >= 170
  { NODE_RESTARY     , NEN_HEAD  , NEN_NONE  , NEN_NONE  , "RESTARY"     }, 
  { NODE_REXPAND     , NEN_HEAD  , NEN_NONE  , NEN_NONE  , "REXPAND"     }, 
#endif
#endif
  { NODE_YIELD       , NEN_STTS  , NEN_NONE  , NEN_NONE  , "YIELD"       }, 
  { NODE_RESCUE      , NEN_HEAD  , NEN_RESQ  , NEN_ELSE  , "RESCUE"      }, 
  { NODE_RESBODY     , NEN_HEAD  , NEN_RESQ  , NEN_ENSR  , "RESBODY"     }, 
  { NODE_ENSURE      , NEN_HEAD  , NEN_ENSR  , NEN_NONE  , "ENSURE"      }, 
  { NODE_AND         , NEN_1ST   , NEN_2ND   , NEN_NONE  , "AND"         }, 
  { NODE_OR          , NEN_1ST   , NEN_2ND   , NEN_NONE  , "OR"          }, 
  { NODE_NOT         , NEN_BODY  , NEN_NONE  , NEN_NONE  , "NOT"         }, 
  { NODE_DOT2        , NEN_BEG   , NEN_END   , NEN_STATE , "DOT2"        }, 
  { NODE_DOT3        , NEN_BEG   , NEN_END   , NEN_STATE , "DOT3"        }, 
  { NODE_FLIP2       , NEN_CNT   , NEN_BEG   , NEN_END   , "FLIP2"       }, 
  { NODE_FLIP3       , NEN_CNT   , NEN_BEG   , NEN_END   , "FLIP3"       }, 
  { NODE_RETURN      , NEN_STTS  , NEN_NONE  , NEN_NONE  , "RETURN"      }, 
  { NODE_ARGS        , NEN_CNT   , NEN_REST  , NEN_OPT   , "ARGS"        }, 
  { NODE_ARGSCAT     , NEN_HEAD  , NEN_BODY  , NEN_NONE  , "ARGSCAT"     }, 
  { NODE_ARGSPUSH    , NEN_HEAD  , NEN_BODY  , NEN_NONE  , "ARGSPUSH"    }, 
  { NODE_CALL        , NEN_RECV  , NEN_ARGS  , NEN_MID   , "CALL"        }, 
  { NODE_FCALL       , NEN_ARGS  , NEN_MID   , NEN_NONE  , "FCALL"       }, 
  { NODE_VCALL       , NEN_MID   , NEN_NONE  , NEN_NONE  , "VCALL"       }, 
  { NODE_SUPER       , NEN_ARGS  , NEN_NONE  , NEN_NONE  , "SUPER"       }, 
  { NODE_ZSUPER      , NEN_ARGS  , NEN_NONE  , NEN_NONE  , "ZSUPER"      }, 
  { NODE_SCOPE       , NEN_RVAL  , NEN_TBL   , NEN_NEXT  , "SCOPE"       }, 
  { NODE_OP_ASGN1    , NEN_RECV  , NEN_ARGS  , NEN_MID   , "OP_ASGN1"    }, 
  { NODE_OP_ASGN2    , NEN_VID   , NEN_RECV  , NEN_VALUE , "OP_ASGN2"    }, 
  { NODE_OP_ASGN_AND , NEN_HEAD  , NEN_VALUE , NEN_NONE  , "OP_ASGN_AND" }, 
  { NODE_OP_ASGN_OR  , NEN_HEAD  , NEN_VALUE , NEN_NONE  , "OP_ASGN_OR"  }, 
  { NODE_MASGN       , NEN_VALUE , NEN_HEAD  , NEN_ARGS  , "MASGN"       }, 
  { NODE_LASGN       , NEN_VID   , NEN_VALUE , NEN_CNT   , "LASGN"       }, 
  { NODE_DASGN       , NEN_VALUE , NEN_VID   , NEN_NONE  , "DASGN"       }, 
  { NODE_DASGN_CURR  , NEN_VALUE , NEN_VID   , NEN_NONE  , "DASGN_CURR"  }, 
  { NODE_GASGN       , NEN_VID   , NEN_VALUE , NEN_ENTRY , "GASGN"       }, 
  { NODE_IASGN       , NEN_VID   , NEN_VALUE , NEN_NONE  , "IASGN"       }, 
  { NODE_CDECL       , NEN_VID   , NEN_VALUE , NEN_NONE  , "CDECL"       }, 
  { NODE_CVDECL      , NEN_VALUE , NEN_VID   , NEN_NONE  , "CVDECL"      }, 
  { NODE_CVASGN      , NEN_VALUE , NEN_VID   , NEN_NONE  , "CVASGN"      }, 
  { NODE_LVAR        , NEN_CNT   , NEN_VID   , NEN_NONE  , "LVAR"        }, 
  { NODE_DVAR        , NEN_VID   , NEN_NONE  , NEN_NONE  , "DVAR"        }, 
  { NODE_GVAR        , NEN_VID   , NEN_ENTRY , NEN_NONE  , "GVAR"        }, 
  { NODE_IVAR        , NEN_VID   , NEN_NONE  , NEN_NONE  , "IVAR"        }, 
  { NODE_CONST       , NEN_VID   , NEN_NONE  , NEN_NONE  , "CONST"       }, 
  { NODE_CVAR        , NEN_VID   , NEN_NONE  , NEN_NONE  , "CVAR"        }, 
  { NODE_CVAR2       , NEN_VID   , NEN_NONE  , NEN_NONE  , "CVAR2"       }, 
  { NODE_BLOCK_ARG   , NEN_CNT   , NEN_NONE  , NEN_NONE  , "BLOCK_ARG"   }, 
  { NODE_COLON2      , NEN_HEAD  , NEN_MID   , NEN_NONE  , "COLON2"      }, 
  { NODE_COLON3      , NEN_MID   , NEN_NONE  , NEN_NONE  , "COLON3"      }, 
  { NODE_NTH_REF     , NEN_NTH   , NEN_NONE  , NEN_NONE  , "NTH_REF"     }, 
  { NODE_BACK_REF    , NEN_NTH   , NEN_NONE  , NEN_NONE  , "BACK_REF"    }, 
  { NODE_HASH        , NEN_HEAD  , NEN_NONE  , NEN_NONE  , "HASH"        }, 
  { NODE_ZARRAY      , NEN_NONE  , NEN_NONE  , NEN_NONE  , "ZARRAY"      }, 
  { NODE_ARRAY       , NEN_ALEN  , NEN_HEAD  , NEN_NEXT  , "ARRAY"       }, 
  { NODE_STR         , NEN_LIT   , NEN_NONE  , NEN_NONE  , "STR"         }, 
  { NODE_NEXT        , NEN_LIT   , NEN_CFLAG , NEN_NONE  , "NEXT"        }, 
#if RUBY_VERSION_CODE >= 170 && RUBY_VERSION_CODE < 180
  { NODE_REGX        , NEN_CFLAG , NEN_NONE  , NEN_NONE  , "REGX"        }, 
#endif
  { NODE_DREGX       , NEN_LIT   , NEN_NEXT  , NEN_CFLAG , "DREGX"       }, 
  { NODE_DREGX_ONCE  , NEN_LIT   , NEN_NEXT  , NEN_CFLAG , "DREGX_ONCE"  }, 
  { NODE_DSTR        , NEN_LIT   , NEN_NEXT  , NEN_NONE  , "DSTR"        }, 
  { NODE_DXSTR       , NEN_LIT   , NEN_NEXT  , NEN_NONE  , "DXSTR"       }, 
  { NODE_XSTR        , NEN_LIT   , NEN_NONE  , NEN_NONE  , "XSTR"        }, 
  { NODE_EVSTR       , NEN_LIT   , NEN_HEAD  , NEN_NEXT  , "EVSTR"       }, 
  { NODE_LIT         , NEN_LIT   , NEN_NONE  , NEN_NONE  , "LIT"         }, 
  { NODE_ATTRSET     , NEN_VID   , NEN_NONE  , NEN_NONE  , "ATTRSET"     }, 
  { NODE_DEFN        , NEN_DEFN  , NEN_MID   , NEN_NOEX  , "DEFN"        }, 
  { NODE_DEFS        , NEN_DEFN  , NEN_RECV  , NEN_MID   , "DEFS"        }, 
  { NODE_UNDEF       , NEN_MID   , NEN_NONE  , NEN_NONE  , "UNDEF"       }, 
  { NODE_ALIAS       , NEN_NEW   , NEN_OLD   , NEN_NONE  , "ALIAS"       }, 
  { NODE_VALIAS      , NEN_NEW   , NEN_OLD   , NEN_NONE  , "VALIAS"      }, 
#if RUBY_VERSION_CODE < 180
  { NODE_CLASS       , NEN_SUPER , NEN_CNAME , NEN_BODY  , "CLASS"       }, 
  { NODE_MODULE      , NEN_CNAME , NEN_BODY  , NEN_NONE  , "MODULE"      }, 
#else
  { NODE_CLASS       , NEN_SUPER , NEN_CPATH , NEN_BODY  , "CLASS"       }, 
  { NODE_MODULE      , NEN_CPATH , NEN_BODY  , NEN_NONE  , "MODULE"      }, 
#endif
  { NODE_SCLASS      , NEN_RECV  , NEN_BODY  , NEN_NONE  , "SCLASS"      }, 
  { NODE_DEFINED     , NEN_HEAD  , NEN_NONE  , NEN_NONE  , "DEFINED"     }, 
  { NODE_NEWLINE     , NEN_NTH   , NEN_NEXT  , NEN_NONE  , "NEWLINE"     }, 
  { NODE_IFUNC       , NEN_CFNC  , NEN_TVAL  , NEN_ARGC  , "IFUNC"       }, 
  { NODE_CFUNC       , NEN_CFNC  , NEN_TVAL  , NEN_ARGC  , "CFUNC"       }, 
  { NODE_FBODY       , NEN_ORIG  , NEN_MID   , NEN_HEAD  , "FBODY"       }, 
  { NODE_CREF        , NEN_CLSS  , NEN_NEXT  , NEN_NONE  , "CREF"        }, 
#ifdef C_ALLOCA
  { NODE_ALLOCA      , NEN_LIT   , NEN_VALUE , NEN_CNT   , "ALLOCA"      }, 
#endif
  { NODE_DMETHOD     , NEN_CVAL  , NEN_NONE  , NEN_NONE  , "DMETHOD"     }, 
  { NODE_BMETHOD     , NEN_CVAL  , NEN_NONE  , NEN_NONE  , "BMETHOD"     }, 
  { NODE_MEMO        , NEN_LIT   , NEN_RVAL  , NEN_NONE  , "MEMO"        }, 
  { NODE_LAST        , NEN_NONE  , NEN_NONE  , NEN_NONE  , "LAST"        }, 
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

