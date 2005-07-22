require 'rbconfig'

class Node
  public

  # Return an string describing this node as a single expression.  By
  # default, this just returns the name of the node's type, but some
  # node types override this method to produce more meaningful output.
  def as_expression(*args)
    return as_expression_impl(self, *args)
  end

  private

  # default
  def as_expression_impl(node)
    return "<#{node.nd_type.to_s}>"
  end

  class << self
    def define_expression(klass, &block)
      if const_defined?(klass) then
        const_get(klass).__send__(:define_method, :as_expression_impl, &block)
      end
    end
  end

  define_expression(:LIT) do |node|
    # TODO: #inspect might not give an eval-able expression
    node.lit.inspect
  end

  define_expression(:FCALL) do |node|
    args = node.args
    "#{node.mid}(#{args ? args.as_expression(false) : ''})"
  end

  define_expression(:VCALL) do |node|
    node.mid.to_s
  end

  @@arithmetic_expressions = [
    :+, :-, :*, :/, :<, :>, :<=, :>=, :==, :===, :<=>, :<<, :>>, :&, :|,
    :^, :%, '!'.intern, '!='.intern
  ]

  # TODO: there should be a way to detect if the expressions need to be
  # in parens
  define_expression(:CALL) do |node|
    case node.mid
    when *@@arithmetic_expressions
      args = node.args
      "(#{node.recv.as_expression}) #{node.mid} (#{args ?  args.as_expression(false) : ''})"
    when :[]
      args = node.args
      "(#{node.recv.as_expression})[#{args ? args.as_expression(false) : ''}]"
    else
      args = node.args
      "(#{node.recv.as_expression}).#{node.mid}(#{args ? args.as_expression(false) : ''})"
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
    def to_a
      a = []
      e = self
      while e do
        a << e.head
        e = e.next
      end
      a
    end

    def as_expression_impl(node, brackets = true)
      s = brackets ? '[' : ''
      s += node.to_a.map { |n| n.as_expression }.join(', ')
      s += brackets ? ']' : ''
      s
    end
  end

  class ZARRAY < Node
    def to_a
      []
    end

    def as_expression_impl(node, brackets = true)
      brackets ? '[]' : ''
    end
  end

  class BLOCK < Node
    def to_a
      a = []
      e = self
      while e do
        a << e.head
        e = e.next
      end
      a
    end

    def as_expression_impl(node)
      a = node.to_a
      d = a[0]
      while d.class == Node::DASGN_CURR do
        d = d.value
      end
      a.shift if not d
      a.map { |n| n.as_expression }.join('; ')
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
    bodynode = node.body.class == Node::NEWLINE ? node.body.next : node.body
    elsenode = node.else.class == Node::NEWLINE ? node.else.next : node.else
    "(#{node.cond.as_expression}) ? " +
    "(#{bodynode.as_expression}) : " +
    "(#{elsenode.as_expression})"
  end

  define_expression(:TRUENODE) do |node|
    "true"
  end

  define_expression(:FALSENODE) do |node|
    "false"
  end

  define_expression(:NILNODE) do |node|
    "nil"
  end

  define_expression(:SELF) do |node|
    "self"
  end

  define_expression(:DOT2) do |node|
    "(#{node.beg.as_expression})..(#{node.end.as_expression})"
  end

  define_expression(:DOT3) do |node|
    "(#{node.beg.as_expression})...(#{node.end.as_expression})"
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
    "#{node.vid} = #{node.value.as_expression}"
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
      "(#{node.recv.as_expression})[#{attrs.join(', ')}] = #{value}"
    else
      "(#{node.recv.as_expression}).#{node.mid}#{node.args.as_expression(false)}"
    end
  end

  define_expression(:CONST) do |node|
    "#{node.vid}"
  end

  define_expression(:COLON2) do |node|
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

    # TODO: not sure how to implement EVSTR on 1.6.x

  end

  define_expression(:ITER) do |node|
    "#{node.iter.as_expression} { #{node.body.as_expression} }"
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
    if node.body.class == Node::RESCUE
      "begin; #{node.body.as_expression(true)}; end"
    elsif node.body
      "begin; #{node.body.as_expression}; end"
    else
      "begin; end"
    end
  end

  define_expression(:ENSURE) do |node|
    if node.head then
      "#{node.head.as_expression} ensure #{node.ensr.as_expression}"
    else
      "ensure #{node.ensr.as_expression}"
    end
  end

  define_expression(:RESCUE) do |node, *args|
    begin_rescue = args[0] || false
    if node.head then
      if begin_rescue then
        "#{node.head.as_expression}; rescue #{node.resq.as_expression(begin_rescue)}"
      else
        "#{node.head.as_expression} rescue #{node.resq.as_expression(begin_rescue)}"
      end
    else
      "rescue #{node.resq.as_expression(begin_rescue)}"
    end
  end

  define_expression(:RESBODY) do |node, *args|
    begin_rescue = args[0] || false
    if begin_rescue then
      if node.ensr then
        a = node.ensr.to_a.map { |n| n.as_expression }
        "#{a.join(', ')}; #{node.resq.as_expression}"
      else
        node.resq ? "; #{node.resq.as_expression}" : ''
      end
    else
      # TODO: assuming node.ensr is false...
      node.resq ? node.resq.as_expression : ''
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
    "alias #{node.mid}"
  end

  define_expression(:CLASS) do |node|
    s_super = node.super ? " < #{node.super.as_expression}" : ''
    "class #{node.cpath.as_expression}#{s_super}; #{node.body.as_expression}; end"
  end

  define_expression(:SCLASS) do |node|
    "class << #{node.recv.as_expression}; #{node.body.as_expression}; end"
  end

  define_expression(:SCOPE) do |node|
    node.next ? node.next.as_expression : ''
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

  # TODO: MATCH3
end

