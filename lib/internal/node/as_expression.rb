require 'internal/node'
require 'internal/node/to_a'
require 'rbconfig'

class Node
  public

  # Return an string describing this node as a single expression.  By
  # default, this just returns the name of the node's type, but some
  # node types override this method to produce more meaningful output.
  def as_expression(*args)
    return as_expression_impl(self, *args)
  end

  # Return a string as with as_expression, but surround it with parens
  # if it is a composite expression, so that it can be used to form more
  # complex expressions.
  def as_paren_expression(*args)
    expr = self.as_expression(*args)
    if not OMIT_PARENS[self.class] then
      expr = "(#{expr})"
    end
    return expr
  end

  private

  # default
  def as_expression_impl(node)
    return "<#{node.nd_type.to_s}>"
  end

  class << self
    def define_expression(klass, &block)
      if const_defined?(klass) then
        const_get(klass).instance_eval { define_method(:as_expression_impl, &block) }
      end
    end
  end

  define_expression(:LIT) do |node|
    # TODO: #inspect might not give an eval-able expression
    node.lit.inspect
  end

  define_expression(:FCALL) do |node|
    # args will either be an ARRAY or ARGSCAT
    args = node.args
    "#{node.mid}(#{args ? args.as_expression(false) : ''})"
  end

  define_expression(:VCALL) do |node|
    node.mid.to_s
  end

  arithmetic_expressions = [
    :+, :-, :*, :/, :<, :>, :<=, :>=, :==, :===, :<=>, :<<, :>>, :&, :|,
    :^, :%, '!'.intern, '!='.intern
  ]

  # TODO: there should be a way to detect if the expressions need to be
  # in parens
  define_expression(:CALL) do |node|
    recv_expr = node.recv.as_paren_expression

    case node.mid
    when *arithmetic_expressions
      args = node.args
      "#{recv_expr} #{node.mid} #{args ? args.as_paren_expression(false) : ''}"
    when :[]
      args = node.args
      "#{recv_expr}[#{args ? args.as_expression(false) : ''}]"
    else
      args = node.args
      "#{recv_expr}.#{node.mid}(#{args ? args.as_expression(false) : ''})"
    end
  end

  define_expression(:ZSUPER) do |node|
    "super"
  end

  define_expression(:SUPER) do |node|
    "super(#{node.args ? node.args.as_expression(false) : ''})"
  end

  define_expression(:REDO) do |node|
    "redo"
  end

  define_expression(:RETRY) do |node|
    "retry"
  end

  define_expression(:NOT) do |node|
    "not #{node.body.as_expression}"
  end

  define_expression(:AND) do |node|
    "#{node.first.as_expression} and #{node.second.as_expression}"
  end

  define_expression(:OR) do |node|
    "#{node.first.as_expression} or #{node.second.as_expression}"
  end

  class ARRAY < Node
    def as_expression_impl(node, brackets = true)
      s = brackets ? '[' : ''
      s << (node.to_a.map { |n| n.as_expression }.join(', '))
      s << (brackets ? ']' : '')
      s
    end
  end

  class ARGSCAT < Node
    def as_expression_impl(node, brackets = true)
      s = brackets ? '[' : ''
      s << node.head.as_expression(false)
      s << ", "
      s << "*#{node.body.as_expression}"
      s << (brackets ? ']' : '')
      s
    end
  end

  class ZARRAY < Node
    def as_expression_impl(node, brackets = true)
      brackets ? '[]' : ''
    end
  end

  class BLOCK < Node
    def as_expression_impl(node)
      a = node.to_a
      if a.size == 1 then
        return 'nil'
      end
      d = a[0]
      while d.class == Node::DASGN_CURR do
        d = d.value
      end
      a.shift if not d
      expressions = a.map { |n| n.as_expression }
      expressions.reject! { |e| e.nil? }
      if expressions.nitems == 0 then
        return 'nil'
      else
        return expressions.join('; ')
      end
    end
  end

  define_expression(:HASH) do |node|
    if not node.head then
      "{}"
    else
      a = node.head.to_a
      elems = []
      i = 0
      while i < a.size do
        elems << "(#{a[i].as_expression})=>(#{a[i+1].as_expression})"
        i += 2
      end
      "{#{elems.join(', ')}}"
    end
  end

  define_expression(:IF) do |node|
    if Node.const_defined?(:NEWLINE) then
      bodynode = node.body.class == Node::NEWLINE ? node.body.next : node.body
      elsenode = node.else.class == Node::NEWLINE ? node.else.next : node.else
    else
      bodynode = node.body
      elsenode = node.else
    end
    if elsenode then
      "#{node.cond.as_paren_expression} ? " +
      "#{bodynode.as_paren_expression} : " +
      "#{elsenode.as_paren_expression}"
    else
      "#{bodynode.as_paren_expression} if " +
      "#{node.cond.as_paren_expression}"
    end
  end

  define_expression(:TRUE) do |node|
    "true"
  end

  define_expression(:FALSE) do |node|
    "false"
  end

  define_expression(:NIL) do |node|
    "nil"
  end

  define_expression(:SELF) do |node|
    "self"
  end

  define_expression(:DOT2) do |node|
    "#{node.beg.as_paren_expression}..#{node.end.as_paren_expression}"
  end

  define_expression(:DOT3) do |node|
    "#{node.beg.as_paren_expression}...#{node.end.as_paren_expression}"
  end

  define_expression(:GVAR) do |node|
    "#{node.vid}"
  end

  define_expression(:IVAR) do |node|
    "#{node.vid}"
  end

  define_expression(:CVAR) do |node|
    "#{node.vid}"
  end

  define_expression(:DVAR) do |node|
    "#{node.vid}"
  end

  define_expression(:NTH_REF) do |node|
    "$#{node.nth}"
  end

  define_expression(:BACK_REF) do |node|
    "$`"
  end

  define_expression(:DASGN_CURR) do |node|
    # node.value is unset for MASGN
    node.value ? "#{node.vid} = #{node.value.as_expression}" : "#{node.vid}"
  end

  define_expression(:DASGN) do |node|
    "#{node.vid} = #{node.value.as_expression}"
  end

  define_expression(:IASGN) do |node|
    "#{node.vid} = #{node.value.as_expression}"
  end

  define_expression(:LASGN) do |node|
    "#{node.vid} = #{node.value.as_expression}"
  end

  define_expression(:MASGN) do |node|
    lhs = node.head.to_a.map { |n| n.as_expression }
    rhs = node.value.to_a.map { |n| n.as_expression }
    "#{lhs.join(', ')} = #{rhs.join(', ')}"
  end

  define_expression(:OP_ASGN1) do |node|
    # TODO
    raise "Not implemented"
  end

  define_expression(:OP_ASGN2) do |node|
    recv = node.recv.as_expression
    attr = node.next.vid # TODO: we assume it's the same as aid
    op = case node.next.mid
    when false then '||'
    when nil then '&&'
    else node.next.mid
    end
    value = node.value.as_expression
    "#{recv}.#{attr} #{op}= #{value}"
  end

  define_expression(:CDECL) do |node|
    "#{node.vid} = #{node.value.as_expression}"
  end

  define_expression(:CVDECL) do |node|
    "#{node.vid} = #{node.value.as_expression}"
  end

  define_expression(:CVASGN) do |node|
    "#{node.vid} = #{node.value.as_expression}"
  end

  define_expression(:ATTRASGN) do |node|
    case node.mid
    when :[]=
      args = node.args.to_a
      attrs = args[1..-2].map { |n| n.as_expression }
      value = args[-1].as_expression
      "#{node.recv.as_paren_expression}[#{attrs.join(', ')}] = #{value}"
    else
      "#{node.recv.as_paren_expression}.#{node.mid}#{node.args.as_expression(false)}"
    end
  end

  define_expression(:CONST) do |node|
    "#{node.vid}"
  end

  define_expression(:COLON2) do |node|
    # TODO: shouldn't COLON2 have args if it's a method?
    if node.head then
      "#{node.head.as_expression}::#{node.mid}"
    else
      node.mid.to_s
    end
  end

  define_expression(:COLON3) do |node|
    "::#{node.mid}"
  end

  define_expression(:LVAR) do |node|
    "#{node.vid}"
  end

  define_expression(:NEWLINE) do |node|
    node.next.as_expression
  end

  define_expression(:STR) do |node|
    "\"#{node.lit.inspect[1..-2]}\""
  end

  define_expression(:REGX) do |node|
    # TODO: cflag
    "/#{node.lit.inspect[1..-2]}/"
  end

  define_expression(:REGX_ONCE) do |node|
    # TODO: cflag
    "/#{node.lit.inspect[1..-2]}/o"
  end

  define_expression(:XSTR) do |node|
    "`#{node.lit.inspect[1..-2]}`"
  end

  define_expression(:DSTR) do |node|
    a = node.next.to_a
    s = "\"#{node.lit.inspect[1..-2]}"
    a.each do |elem|
      case elem
      when Node::STR then s += elem.lit
      else s += elem.as_expression
      end
    end
    s += "\""
    s
  end

  define_expression(:DREGX) do |node|
    a = node.next.to_a
    s = "/#{node.lit.inspect[1..-2]}"
    a.each do |elem|
      case elem
      when Node::STR then s += elem.lit
      else s += elem.as_expression
      end
    end
    s += "/"
    # TODO: cflag
    s
  end

  define_expression(:DREGX_ONCE) do |node|
    a = node.next.to_a
    s = "/#{node.lit.inspect[1..-2]}"
    a.each do |elem|
      case elem
      when Node::STR then s += elem.lit
      else s += elem.as_expression
      end
    end
    s += "/o"
    # TODO: cflag
    s
  end

  define_expression(:DXSTR) do |node|
    a = node.next.to_a
    s = "`#{node.lit.inspect[1..-2]}"
    a.each do |elem|
      case elem
      when Node::STR then s += elem.lit
      else s += elem.as_expression
      end
    end
    s += "`"
    s
  end

  major = Config::CONFIG['MAJOR'].to_i
  minor = Config::CONFIG['MINOR'].to_i
  teeny = Config::CONFIG['TEENY'].to_i
  ruby_version_code = major * 100 + minor * 10 + teeny

  if ruby_version_code >= 180 then

    define_expression(:EVSTR) do |node|
      "\#\{#{node.body.as_expression}\}"
    end

  else

    define_expression(:EVSTR) do |node|
      "\#\{#{node.lit}\}"
    end

  end

  define_expression(:ITER) do |node|
    iter = node.iter.as_expression
    body = node.body ? (node.body.as_expression + " ") : ""
    "#{iter} { #{body} }"
  end

  define_expression(:WHILE) do |node|
    if node.state == 1 then
      "while #{node.cond.as_expression} do; #{node.body.as_expression}; end"
    else
      "begin; #{node.body.as_expression}; end while #{node.cond.as_expression}"
    end
  end

  define_expression(:UNTIL) do |node|
    if node.state == 1 then
      "until #{node.cond.as_expression} do; #{node.body.as_expression}; end"
    else
      "begin; #{node.body.as_expression}; end until #{node.cond.as_expression}"
    end
  end

  define_expression(:BREAK) do |node|
    s = "break"
    if node.stts then
      s += " #{node.stts.as_expression}"
    end
    s
  end

  define_expression(:RETURN) do |node|
    s = "return"
    if node.stts then
      s += " #{node.stts.as_expression}"
    end
    s
  end

  define_expression(:YIELD) do |node|
    s = "yield"
    if node.stts then
      s += " #{node.stts.as_expression}"
    end
    s
  end

  define_expression(:BEGIN) do |node|
    if node.body.class == Node::RESCUE or
       node.body.class == Node::ENSURE then
      "begin; #{node.body.as_expression(true)}; end"
    elsif node.body then
      "begin; #{node.body.as_expression}; end"
    else
      "begin; end"
    end
  end

  define_expression(:ENSURE) do |node, *args|
    begin_ensure = args[0] || false
    if node.head then
      if begin_ensure then
        "#{node.head.as_expression} ensure #{node.ensr.as_expression}"
      else
        "begin; #{node.head.as_expression} ensure #{node.ensr.as_expression}; end"
      end
    else
      if begin_ensure then
        "ensure #{node.ensr.as_expression}"
      else
        "begin; ensure #{node.ensr.as_expression}; end"
      end
    end
  end

  define_expression(:RESCUE) do |node, *args|
    begin_rescue = args[0] || false
    if node.head then
      if begin_rescue then
        "#{node.head.as_expression}; rescue #{node.resq.as_expression(begin_rescue)}"
      else
        if not node.resq or not node.resq.body then
          "begin; #{node.head.as_expression}; rescue; end"
        else
          "#{node.head.as_paren_expression} rescue #{node.resq.as_expression(begin_rescue)}"
        end
      end
    else
      if not node.resq or not node.resq.body then
        "begin; rescue; end"
      else
        "rescue #{node.resq.as_expression(begin_rescue)}"
      end
    end
  end

  define_expression(:RESBODY) do |node, *args|
    begin_rescue = args[0] || false
    if begin_rescue then
      if node.args then
        a = node.args.to_a.map { |n| n.as_expression }
        "#{a.join(', ')}; #{node.resq.as_expression}"
      else
        node.body ? "; #{node.body.as_expression}" : ''
      end
    else
      # TODO: assuming node.args is false...
      node.body ? node.body.as_expression : ''
    end
  end

  define_expression(:CASE) do |node|
    "case #{node.head.as_expression}; #{node.body.as_expression}end"
  end

  define_expression(:WHEN) do |node|
    args = node.head.to_a.map { |n| n.as_expression }
    s = ''
    if node.body then
      s = "when #{args.join(', ')} then #{node.body.as_expression}; "
    else
      s = "when #{args.join(', ')}; "
    end
    if node.next then
      s += node.next.as_expression
    end
    s
  end

  define_expression(:ALIAS) do |node|
    "alias #{node.new} #{node.old}"
  end

  define_expression(:VALIAS) do |node|
    "alias #{node.new} #{node.old}"
  end

  define_expression(:UNDEF) do |node|
    if defined? node.mid then
      # < 1.8.5
      "undef #{node.mid}"
    else
      # >= 1.8.5
      "undef #{node.body.as_expression}"
    end
  end

  define_expression(:CLASS) do |node|
    s_super = node.super ? " < #{node.super.as_expression}" : ''
    if node.respond_to?(:cpath) then
      path = node.cpath.as_expression
    else
      path = node.cname
    end
    "class #{path}#{s_super}; #{node.body.as_expression}; end"
  end

  define_expression(:SCLASS) do |node|
    "class << #{node.recv.as_expression}; #{node.body.as_expression}; end"
  end

  define_expression(:SCOPE) do |node|
    case node.next
    when nil then ''
    when Node::ARGS then 'nil'
    when Node::BLOCK_ARG then 'nil'
    else node.next.as_expression
    end
  end

  define_expression(:DEFN) do |node|
    # TODO: what to do about noex?
    "def #{node.mid}; #{node.next.as_expression}; end"
  end

  define_expression(:DEFS) do |node|
    "def #{node.recv.as_expression}.#{node.mid}; #{node.next.as_expression}; end"
  end

  define_expression(:DEFINED) do |node|
    "defined?(#{node.head.as_expression})"
  end

  define_expression(:ARGS) do |node|
    nil
  end

  define_expression(:BLOCK_ARG) do |node|
    nil
  end

  # TODO: MATCH3

  OMIT_PARENS = {
    LVAR   => true,
    GVAR   => true,
    IVAR   => true,
    CVAR   => true,
    DVAR   => true,
    LIT    => true,
    ARRAY  => true,
    ZARRAY => true,
    HASH   => true,
  }
end

