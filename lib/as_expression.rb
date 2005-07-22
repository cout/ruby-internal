require 'rbconfig'

class Node
  # Return an string describing this node as a single expression.  By
  # default, this just returns the name of the node's type, but some
  # node types override this method to produce more meaningful output.
  def as_expression
    # default
    return "<#{self.nd_type.to_s}>"
  end

  class << self
    def define_expression(klass, &block)
      if const_defined?(klass) then
        const_get(klass).__send__(:define_method, :as_expression, &block)
      end
    end
  end

  define_expression(:LIT) do
    # TODO: #inspect might not give an eval-able expression
    self.lit.inspect
  end

  define_expression(:FCALL) do
    args = self.args
    "#{self.mid}(#{args ? args.as_expression(false) : ''})"
  end

  define_expression(:VCALL) do
    self.mid.to_s
  end

  @@arithmetic_expressions = [
    :+, :-, :*, :/, :<, :>, :<=, :>=, :==, :===, :<=>, :<<, :>>, :&, :|,
    :^, :%, '!'.intern, '!='.intern
  ]

  # TODO: there should be a way to detect if the expressions need to be
  # in parens
  define_expression(:CALL) do
    case self.mid
    when *@@arithmetic_expressions
      args = self.args
      "(#{self.recv.as_expression}) #{self.mid} (#{args ?  args.as_expression(false) : ''})"
    when :[]
      args = self.args
      "(#{self.recv.as_expression})[#{args ? args.as_expression(false) : ''}]"
    else
      args = self.args
      "(#{self.recv.as_expression}).#{self.mid}(#{args ? args.as_expression(false) : ''})"
    end
  end

  define_expression(:ZSUPER) do
    "super"
  end

  define_expression(:SUPER) do
    "super(#{self.args ? self.args.as_expression(false) : ''})"
  end

  define_expression(:REDO) do
    "redo"
  end

  define_expression(:RETRY) do
    "retry"
  end

  define_expression(:NOT) do
    "not #{self.body.as_expression}"
  end

  define_expression(:AND) do
    "#{self.first.as_expression} and #{self.second.as_expression}"
  end

  define_expression(:OR) do
    "#{self.first.as_expression} or #{self.second.as_expression}"
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

    def as_expression(brackets = true)
      s = brackets ? '[' : ''
      s += self.to_a.map { |n| n.as_expression }.join(', ')
      s += brackets ? ']' : ''
      s
    end
  end

  class ZARRAY < Node
    def to_a
      []
    end

    def as_expression(brackets = true)
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

    def as_expression
      a = self.to_a
      d = a[0]
      while d.class == Node::DASGN_CURR do
        d = d.value
      end
      a.shift if not d
      a.map { |n| n.as_expression }.join('; ')
    end
  end

  define_expression(:HASH) do
    if not self.head then
      "{}"
    else
      a = self.head.to_a
      elems = []
      i = 0
      while i < a.size do
        elems << "(#{a[i].as_expression})=>(#{a[i+1].as_expression})"
        i += 2
      end
      "{#{elems.join(', ')}}"
    end
  end

  define_expression(:IF) do
    bodynode = self.body.class == Node::NEWLINE ? self.body.next : self.body
    elsenode = self.else.class == Node::NEWLINE ? self.else.next : self.else
    "(#{self.cond.as_expression}) ? " +
    "(#{bodynode.as_expression}) : " +
    "(#{elsenode.as_expression})"
  end

  define_expression(:TRUENODE) do
    "true"
  end

  define_expression(:FALSENODE) do
    "false"
  end

  define_expression(:NILNODE) do
    "nil"
  end

  define_expression(:SELF) do
    "self"
  end

  define_expression(:DOT2) do
    "(#{self.beg.as_expression})..(#{self.end.as_expression})"
  end

  define_expression(:DOT3) do
    "(#{self.beg.as_expression})...(#{self.end.as_expression})"
  end

  define_expression(:GVAR) do
    "#{self.vid}"
  end

  define_expression(:IVAR) do
    "#{self.vid}"
  end

  define_expression(:CVAR) do
    "#{self.vid}"
  end

  define_expression(:DVAR) do
    "#{self.vid}"
  end

  define_expression(:NTH_REF) do
    "$#{self.nth}"
  end

  define_expression(:BACK_REF) do
    "$`"
  end

  define_expression(:DASGN_CURR) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:DASGN) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:IASGN) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:LASGN) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:MASGN) do
    lhs = self.head.to_a.map { |n| n.as_expression }
    rhs = self.value.to_a.map { |n| n.as_expression }
    "#{lhs.join(', ')} = #{rhs.join(', ')}"
  end

  define_expression(:CDECL) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:CVDECL) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:CVASGN) do
    "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:ATTRASGN) do
    case self.mid
    when :[]=
      args = self.args.to_a
      attrs = args[1..-2].map { |n| n.as_expression }
      value = args[-1].as_expression
      "(#{self.recv.as_expression})[#{attrs.join(', ')}] = #{value}"
    else
      "(#{self.recv.as_expression}).#{self.mid}#{self.args.as_expression(false)}"
    end
  end

  define_expression(:CONST) do
    "#{self.vid}"
  end

  define_expression(:COLON2) do
    if self.head then
      "#{self.head.as_expression}::#{self.mid}"
    else
      self.mid.to_s
    end
  end

  define_expression(:COLON3) do
    "::#{self.mid}"
  end

  define_expression(:LVAR) do
    "#{self.vid}"
  end

  define_expression(:NEWLINE) do
    self.next.as_expression
  end

  define_expression(:STR) do
    "\"#{self.lit.inspect[1..-2]}\""
  end

  define_expression(:REGX) do
    # TODO: cflag
    "/#{self.lit.inspect[1..-2]}/"
  end

  define_expression(:REGX_ONCE) do
    # TODO: cflag
    "/#{self.lit.inspect[1..-2]}/o"
  end

  define_expression(:XSTR) do
    "`#{self.lit.inspect[1..-2]}`"
  end

  define_expression(:DSTR) do
    a = self.next.to_a
    s = "\"#{self.lit.inspect[1..-2]}"
    a.each do |elem|
      case elem
      when Node::STR then s += elem.lit
      else s += elem.as_expression
      end
    end
    s += "\""
    s
  end

  define_expression(:DREGX) do
    a = self.next.to_a
    s = "/#{self.lit.inspect[1..-2]}"
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

  define_expression(:DREGX_ONCE) do
    a = self.next.to_a
    s = "/#{self.lit.inspect[1..-2]}"
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

  define_expression(:DXSTR) do
    a = self.next.to_a
    s = "`#{self.lit.inspect[1..-2]}"
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

    define_expression(:EVSTR) do
      "\#\{#{self.body.as_expression}\}"
    end

  else

    # TODO: not sure how to implement EVSTR on 1.6.x

  end

  define_expression(:ITER) do
    "#{self.iter.as_expression} { #{self.body.as_expression} }"
  end

  define_expression(:WHILE) do
    if self.state == 1 then
      "while #{self.cond.as_expression} do; #{self.body.as_expression}; end"
    else
      "begin; #{self.body.as_expression}; end while #{self.cond.as_expression}"
    end
  end

  define_expression(:UNTIL) do
    if self.state == 1 then
      "until #{self.cond.as_expression} do; #{self.body.as_expression}; end"
    else
      "begin; #{self.body.as_expression}; end until #{self.cond.as_expression}"
    end
  end

  define_expression(:BREAK) do
    s = "break"
    if self.stts then
      s += " #{self.stts.as_expression}"
    end
    s
  end

  define_expression(:RETURN) do
    s = "return"
    if self.stts then
      s += " #{self.stts.as_expression}"
    end
    s
  end

  define_expression(:YIELD) do
    s = "yield"
    if self.stts then
      s += " #{self.stts.as_expression}"
    end
    s
  end

  define_expression(:BEGIN) do
    if self.body.class == Node::RESCUE
      "begin; #{self.body.as_expression(true)}; end"
    elsif self.body
      "begin; #{self.body.as_expression}; end"
    else
      "begin; end"
    end
  end

  define_expression(:ENSURE) do
    if self.head then
      "#{self.head.as_expression} ensure #{self.ensr.as_expression}"
    else
      "ensure #{self.ensr.as_expression}"
    end
  end

  define_expression(:RESCUE) do |*args|
    begin_rescue = args[0] || false
    if self.head then
      if begin_rescue then
        "#{self.head.as_expression}; rescue #{self.resq.as_expression(begin_rescue)}"
      else
        "#{self.head.as_expression} rescue #{self.resq.as_expression(begin_rescue)}"
      end
    else
      "rescue #{self.resq.as_expression(begin_rescue)}"
    end
  end

  define_expression(:RESBODY) do |*args|
    begin_rescue = args[0] || false
    if begin_rescue then
      if self.ensr then
        a = self.ensr.to_a.map { |n| n.as_expression }
        "#{a.join(', ')}; #{self.resq.as_expression}"
      else
        self.resq ? "; #{self.resq.as_expression}" : ''
      end
    else
      # TODO: assuming self.ensr is false...
      self.resq ? self.resq.as_expression : ''
    end
  end

  define_expression(:CASE) do
    "case #{self.head.as_expression}; #{self.body.as_expression}end"
  end

  define_expression(:WHEN) do
    args = self.head.to_a.map { |n| n.as_expression }
    s = ''
    if self.body then
      s = "when #{args.join(', ')} then #{self.body.as_expression}; "
    else
      s = "when #{args.join(', ')}; "
    end
    if self.next then
      s += self.next.as_expression
    end
    s
  end

  define_expression(:ALIAS) do
    "alias #{self.new} #{self.old}"
  end

  define_expression(:VALIAS) do
    "alias #{self.new} #{self.old}"
  end

  define_expression(:UNDEF) do
    "alias #{self.mid}"
  end

  define_expression(:CLASS) do
    s_super = self.super ? " < #{self.super.as_expression}" : ''
    "class #{self.cpath.as_expression}#{s_super}; #{self.body.as_expression}; end"
  end

  define_expression(:SCLASS) do
    "class << #{self.recv.as_expression}; #{self.body.as_expression}; end"
  end

  define_expression(:SCOPE) do
    self.next ? self.next.as_expression : ''
  end

  define_expression(:DEFN) do
    # TODO: what to do about noex?
    "def #{self.mid}; #{self.next.as_expression}; end"
  end

  define_expression(:DEFS) do
    "def #{self.recv.as_expression}.#{self.mid}; #{self.next.as_expression}; end"
  end

  define_expression(:DEFINED) do
    "defined?(#{self.head.as_expression})"
  end

  # TODO: MATCH3
end

