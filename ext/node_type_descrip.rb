require 'ruby_version_code'
require 'ruby_source_dir'

NODE_TYPE_DESCRIPS = [
  [ 'BLOCK'       , 'HEAD'  , 'NEXT'  , 'NONE'  ], 
  [ 'POSTEXE'     , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'BEGIN'       , 'BODY'  , 'NONE'  , 'NONE'  ], 
  [ 'MATCH'       , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'MATCH2'      , 'RECV'  , 'VALUE' , 'NONE'  ], 
  [ 'MATCH3'      , 'RECV'  , 'VALUE' , 'NONE'  ], 
  [ 'OPT_N'       , 'BODY'  , 'NONE'  , 'NONE'  ], 
  [ 'SELF'        , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'NIL'         , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'TRUE'        , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'FALSE'       , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'IF'          , 'BODY'  , 'ELSE'  , 'COND'  ], 
  [ 'WHEN'        , 'HEAD'  , 'BODY'  , 'NEXT'  ], 
  [ 'CASE'        , 'HEAD'  , 'BODY'  , 'NEXT'  ], 
  [ 'WHILE'       , 'COND'  , 'BODY'  , 'STATE' ], 
  [ 'UNTIL'       , 'COND'  , 'BODY'  , 'STATE' ], 
  [ 'BLOCK_PASS'  , 'BODY'  , 'ITER'  , 'NONE'  ], 
  [ 'ITER'        , 'VAR'   , 'BODY'  , 'ITER'  ], 
  [ 'FOR'         , 'VAR'   , 'BODY'  , 'ITER'  ], 
  [ 'BREAK'       , 'STTS'  , 'NONE'  , 'NONE'  ], 
  [ 'NEXT'        , 'STTS'  , 'NONE'  , 'NONE'  ], 
  [ 'REDO'        , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'RETRY'       , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'YIELD'       , 'STTS'  , 'NONE'  , 'NONE'  ], 
  [ 'RESCUE'      , 'HEAD'  , 'RESQ'  , 'ELSE'  ], 
  [ 'RESBODY'     , 'HEAD'  , 'RESQ'  , 'ENSR'  ], 
  [ 'ENSURE'      , 'HEAD'  , 'ENSR'  , 'NONE'  ], 
  [ 'AND'         , '1ST'   , '2ND'   , 'NONE'  ], 
  [ 'OR'          , '1ST'   , '2ND'   , 'NONE'  ], 
  [ 'NOT'         , 'BODY'  , 'NONE'  , 'NONE'  ], 
  [ 'DOT2'        , 'BEG'   , 'END'   , 'STATE' ], 
  [ 'DOT3'        , 'BEG'   , 'END'   , 'STATE' ], 
  [ 'FLIP2'       , 'CNT'   , 'BEG'   , 'END'   ], 
  [ 'FLIP3'       , 'CNT'   , 'BEG'   , 'END'   ], 
  [ 'RETURN'      , 'STTS'  , 'NONE'  , 'NONE'  ], 
  [ 'ARGS'        , 'CNT'   , 'REST'  , 'OPT'   ], 
  [ 'ARGSCAT'     , 'HEAD'  , 'BODY'  , 'NONE'  ], 
  [ 'ARGSPUSH'    , 'HEAD'  , 'BODY'  , 'NONE'  ], 
  [ 'CALL'        , 'RECV'  , 'ARGS'  , 'MID'   ], 
  [ 'FCALL'       , 'ARGS'  , 'MID'   , 'NONE'  ], 
  [ 'VCALL'       , 'MID'   , 'NONE'  , 'NONE'  ], 
  [ 'SUPER'       , 'ARGS'  , 'NONE'  , 'NONE'  ], 
  [ 'ZSUPER'      , 'ARGS'  , 'NONE'  , 'NONE'  ], 
  # rval holds an object, not a node
  [ 'SCOPE'       , 'RVAL'  , 'TBL'   , 'NEXT'  ], 
  [ 'OP_ASGN1'    , 'RECV'  , 'ARGS'  , 'MID'   ], 
  [ 'OP_ASGN2'    , 'VID'   , 'RECV'  , 'VALUE' ], 
  [ 'OP_ASGN_AND' , 'HEAD'  , 'VALUE' , 'NONE'  ], 
  [ 'OP_ASGN_OR'  , 'HEAD'  , 'VALUE' , 'AID'   ], 
  [ 'MASGN'       , 'VALUE' , 'HEAD'  , 'ARGS'  ], 
  [ 'LASGN'       , 'VID'   , 'VALUE' , 'CNT'   ], 
  [ 'DASGN'       , 'VALUE' , 'VID'   , 'NONE'  ], 
  [ 'DASGN_CURR'  , 'VALUE' , 'VID'   , 'NONE'  ], 
  [ 'GASGN'       , 'VID'   , 'VALUE' , 'ENTRY' ], 
  [ 'IASGN'       , 'VID'   , 'VALUE' , 'NONE'  ], 
  [ 'CDECL'       , 'VID'   , 'VALUE' , 'NONE'  ], 
  [ 'CVDECL'      , 'VALUE' , 'VID'   , 'NONE'  ], 
  [ 'CVASGN'      , 'VALUE' , 'VID'   , 'NONE'  ], 
  [ 'LVAR'        , 'CNT'   , 'VID'   , 'NONE'  ], 
  [ 'DVAR'        , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'GVAR'        , 'VID'   , 'ENTRY' , 'NONE'  ], 
  [ 'IVAR'        , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'CONST'       , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'CVAR'        , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'BLOCK_ARG'   , 'CNT'   , 'NONE'  , 'NONE'  ], 
  [ 'COLON2'      , 'HEAD'  , 'MID'   , 'NONE'  ], 
  [ 'COLON3'      , 'MID'   , 'NONE'  , 'NONE'  ], 
  [ 'NTH_REF'     , 'NTH'   , 'CNT'   , 'NONE'  ], 
  [ 'BACK_REF'    , 'NTH'   , 'CNT'   , 'NONE'  ], 
  [ 'HASH'        , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'ZARRAY'      , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'ARRAY'       , 'ALEN'  , 'HEAD'  , 'NEXT'  ], 
  [ 'STR'         , 'LIT'   , 'NONE'  , 'NONE'  ], 
  [ 'DREGX'       , 'LIT'   , 'NEXT'  , 'CFLAG' ], 
  [ 'DREGX_ONCE'  , 'LIT'   , 'NEXT'  , 'CFLAG' ], 
  [ 'DSTR'        , 'LIT'   , 'NEXT'  , 'NONE'  ], 
  [ 'DXSTR'       , 'LIT'   , 'NEXT'  , 'NONE'  ], 
  [ 'XSTR'        , 'LIT'   , 'NONE'  , 'NONE'  ], 
  [ 'ALLOCA'      , 'LIT'   , 'VALUE' , 'CNT'   ], 
  [ 'LIT'         , 'LIT'   , 'NONE'  , 'NONE'  ], 
  [ 'ATTRSET'     , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'DEFN'        , 'DEFN'  , 'MID'   , 'NOEX'  ], 
  [ 'DEFS'        , 'DEFN'  , 'RECV'  , 'MID'   ], 
  [ 'UNDEF'       , 'MID'   , 'NONE'  , 'NONE'  ], 
  [ 'ALIAS'       , '1ST'   , '2ND'   , 'NONE'  ], 
  [ 'VALIAS'      , '1ST'   , '2ND'   , 'NONE'  ], 
  [ 'SCLASS'      , 'RECV'  , 'BODY'  , 'NONE'  ], 
  [ 'DEFINED'     , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'IFUNC'       , 'CFNC'  , 'TVAL'  , 'ARGC'  ], 
  [ 'CFUNC'       , 'CFNC'  , 'TVAL'  , 'ARGC'  ], 
  [ 'CREF'        , 'CLSS'  , 'NEXT'  , 'BODY'  ], 
  [ 'BMETHOD'     , 'CVAL'  , 'NONE'  , 'NONE'  ], 
  [ 'MEMO'        , 'LIT'   , 'TVAL'  , 'NONE'  ], 
]
if RUBY_VERSION_CODE < 190 then
# (0, 190)
NODE_TYPE_DESCRIPS.concat [
  [ 'DMETHOD'     , 'CVAL'  , 'NONE'  , 'NONE'  ], 
  [ 'NEWLINE'     , 'NTH'   , 'NEXT'  , 'NONE'  ], 
  [ 'METHOD'      , 'NOEX'  , 'BODY'  , 'NONE'  ], 
  [ 'FBODY'       , 'ORIG'  , 'MID'   , 'HEAD'  ], 
]
else
# [190, oo)
NODE_TYPE_DESCRIPS.concat [
  [ 'METHOD'      , 'BODY'  , 'CLSS'  , 'NOEX'  ], 
  [ 'FBODY'       , 'BODY'  , 'OID'   , 'CNT'  ], 
]
end
if RUBY_VERSION_CODE < 180 then
# (0, 180)
NODE_TYPE_DESCRIPS.concat [
  [ 'CVAR2'       , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'RESTARGS'    , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'CLASS'       , 'SUPER' , 'CNAME' , 'BODY'  ], 
  [ 'MODULE'      , 'CNAME' , 'BODY'  , 'NONE'  ], 
  [ 'EVSTR'       , 'LIT'   , 'NONE'  , 'NONE'  ], 
]
if RUBY_VERSION_CODE >= 170 then
# [170, oo)
NODE_TYPE_DESCRIPS.concat [
  [ 'RESTARY'     , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'REXPAND'     , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'REGX'        , 'CFLAG' , 'NONE'  , 'NONE'  ], 
]
end
else # RUBY_VERSION_CODE >= 180
# [180, oo)
NODE_TYPE_DESCRIPS.concat [
  [ 'CLASS'       , 'SUPER' , 'CPATH' , 'BODY'  ], 
  [ 'MODULE'      , 'CPATH' , 'BODY'  , 'NONE'  ], 
  [ 'DSYM'        , 'LIT'   , 'NEXT'  , 'NONE'  ],
  [ 'ATTRASGN'    , 'MID'   , 'RECV'  , 'ARGS'  ],
  [ 'EVSTR'       , 'BODY'  , 'NONE'  , 'NONE'  ], 
  [ 'TO_ARY'      , 'HEAD'  , 'NONE'  , 'NONE'  ],
  [ 'SPLAT'       , 'HEAD'  , 'NONE'  , 'NONE'  ],
]
end


# *** MUST BE LAST ***
NODE_TYPE_DESCRIPS.concat [
  [ 'LAST'        , 'NONE'  , 'NONE'  , 'NONE'  ], 
]


Node_Type_Descrip = Struct.new(:name, :node1, :node2, :node3)

NODE_TYPE_DESCRIPS.map! { |x| Node_Type_Descrip.new(*x) }

NEN_TO_NODE_TYPE = {}
NODE_TYPE_DESCRIPS.each do |descrip|
  [descrip.node1, descrip.node2, descrip.node3].each do |nen|
    NEN_TO_NODE_TYPE[nen] ||= []
    NEN_TO_NODE_TYPE[nen] << descrip.name
  end
end

