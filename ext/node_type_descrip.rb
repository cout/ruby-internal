require 'rbconfig'
require 'ruby_version_code'
require 'ruby_source_dir'

C_ALLOCA = false
config_h_location = "#{RUBY_SOURCE_DIR}/config.h"
File.open(config_h_location) do |config_h|
  config_h.each_line do |line|
    if line =~ /#define\s+C_ALLOCA\s+1/ then
      C_ALLOCA = true
    end
  end
end

NODE_TYPE_DESCRIPS = [
  [ 'METHOD'      , 'NOEX'  , 'BODY'  , 'CNT'   ], 
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
  [ 'OP_ASGN_OR'  , 'HEAD'  , 'VALUE' , 'NONE'  ], 
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
  [ 'BACK_REF'    , 'NTH'   , 'NONE'  , 'NONE'  ], 
  [ 'HASH'        , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'ZARRAY'      , 'NONE'  , 'NONE'  , 'NONE'  ], 
  [ 'ARRAY'       , 'ALEN'  , 'HEAD'  , 'NEXT'  ], 
  [ 'STR'         , 'LIT'   , 'NONE'  , 'NONE'  ], 
  [ 'DREGX'       , 'LIT'   , 'NEXT'  , 'CFLAG' ], 
  [ 'DREGX_ONCE'  , 'LIT'   , 'NEXT'  , 'CFLAG' ], 
  [ 'DSTR'        , 'LIT'   , 'NEXT'  , 'NONE'  ], 
  [ 'DXSTR'       , 'LIT'   , 'NEXT'  , 'NONE'  ], 
  [ 'XSTR'        , 'LIT'   , 'NONE'  , 'NONE'  ], 
]
if RUBY_VERSION_CODE < 180 then
NODE_TYPE_DESCRIPS.concat [
  [ 'EVSTR'       , 'LIT'   , 'HEAD'  , 'NEXT'  ], 
]
else
NODE_TYPE_DESCRIPS.concat [
  [ 'EVSTR'       , 'BODY'  , 'NONE'  , 'NONE'  ], 
]
end
NODE_TYPE_DESCRIPS.concat [
  [ 'LIT'         , 'LIT'   , 'NONE'  , 'NONE'  ], 
  [ 'ATTRSET'     , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'DEFN'        , 'DEFN'  , 'MID'   , 'NOEX'  ], 
  [ 'DEFS'        , 'DEFN'  , 'RECV'  , 'MID'   ], 
  [ 'UNDEF'       , 'MID'   , 'NONE'  , 'NONE'  ], 
  [ 'ALIAS'       , 'NEW'   , 'OLD'   , 'NONE'  ], 
  [ 'VALIAS'      , 'NEW'   , 'OLD'   , 'NONE'  ], 
  [ 'SCLASS'      , 'RECV'  , 'BODY'  , 'NONE'  ], 
  [ 'DEFINED'     , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'IFUNC'       , 'CFNC'  , 'TVAL'  , 'ARGC'  ], 
  [ 'CFUNC'       , 'CFNC'  , 'TVAL'  , 'ARGC'  ], 
  [ 'FBODY'       , 'ORIG'  , 'MID'   , 'HEAD'  ], 
  [ 'CREF'        , 'HEAD'  , 'NEXT'  , 'BODY'  ], 
  [ 'BMETHOD'     , 'CVAL'  , 'NONE'  , 'NONE'  ], 
  [ 'MEMO'        , 'LIT'   , 'TVAL'  , 'NONE'  ], 
]
if RUBY_VERSION_CODE < 190 then
NODE_TYPE_DESCRIPS.concat [
  [ 'DMETHOD'     , 'CVAL'  , 'NONE'  , 'NONE'  ], 
  [ 'NEWLINE'     , 'NTH'   , 'NEXT'  , 'NONE'  ], 
]
end
if RUBY_VERSION_CODE < 180 then
NODE_TYPE_DESCRIPS.concat [
  [ 'CVAR2'       , 'VID'   , 'NONE'  , 'NONE'  ], 
  [ 'RESTARGS'    , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'CLASS'       , 'SUPER' , 'CNAME' , 'BODY'  ], 
  [ 'MODULE'      , 'CNAME' , 'BODY'  , 'NONE'  ], 
]
if RUBY_VERSION_CODE >= 170 then
NODE_TYPE_DESCRIPS.concat [
  [ 'RESTARY'     , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'REXPAND'     , 'HEAD'  , 'NONE'  , 'NONE'  ], 
  [ 'REGX'        , 'CFLAG' , 'NONE'  , 'NONE'  ], 
]
end
else # RUBY_VERSION_CODE >= 180
NODE_TYPE_DESCRIPS.concat [
  [ 'CLASS'       , 'SUPER' , 'CPATH' , 'BODY'  ], 
  [ 'MODULE'      , 'CPATH' , 'BODY'  , 'NONE'  ], 
  [ 'DSYM'        , 'LIT'   , 'NEXT'  , 'NONE'  ],
  [ 'ATTRASGN'    , 'MID'   , 'RECV'  , 'ARGS'  ],
]
end
if C_ALLOCA then
NODE_TYPE_DESCRIPS.concat [
  [ 'ALLOCA'      , 'LIT'   , 'VALUE' , 'CNT'   ], 
]
end
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

