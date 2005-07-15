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
    return self.lit.to_s
  end

  define_expression(:FCALL) do
    args = self.args
    return "#{self.mid}(#{args ? args.as_expression(false) : ''})"
  end

  define_expression(:VCALL) do
    args = self.args
    return "#{self.mid}(#{args ? args.as_expression(false) : ''})"
  end

  @@arithmetic_expressions = [
    :+, :-, :*, :/, :<, :>, :<=, :>=, :==, :===, :<=>, :<<, :>>, :&, :|,
    :^, :%, '!'.intern, '!='.intern, '=~'.intern, ':!~'.intern
  ]

  # TODO: there should be a way to detect if the expressions need to be
  # in parens
  define_expression(:CALL) do
    case self.mid
    when *@@arithmetic_expressions
      args = self.args
      return "(#{self.recv.as_expression}) #{self.mid} (#{args ?  args.as_expression(false) : ''})"
    when :[]
      args = self.args
      return "(#{self.recv.as_expression})[#{args ? args.as_expression(false) : ''}]"
    else
      args = self.args
      return "(#{self.recv.as_expression}).#{self.mid}(#{args ? args.as_expression(false) : ''})"
    end
  end

  define_expression(:VCALL) do
    return self.mid.to_s
  end

  define_expression(:ZSUPER) do
    return "super"
  end

  define_expression(:SUPER) do
    return "super(#{args ? args.as_expression(false) : ''})"
  end

  class ARRAY < Node
    def to_a
      a = []
      e = self
      while e do
        a << e.head
        e = e.next
      end
      return a
    end

    def as_expression(brackets = true)
      s = brackets ? '[' : ''
      s += self.to_a.map { |n| n.as_expression }.join(', ')
      s += brackets ? ']' : ''
      return s
    end
  end

  class ZARRAY < Node
    def to_a
      return []
    end

    def as_expression(brackets = true)
      return brackets ? '[]' : ''
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
      return a
    end

    def as_expression
      a = self.to_a
      if a[0].class == Node::DASGN_CURR and not a[0].value
        # ignore variable definitions
        a.shift
      end
      return a.map { |n| n.as_expression }.join('; ')
    end
  end

  define_expression(:HASH) do
    a = self.head.to_a
    elems = []
    i = 0
    while i < a.size do
      elems << "(#{a[i].as_expression})=>(#{a[i+1].as_expression})"
      i += 2
    end
    return "{#{elems.join(', ')}}"
  end

  define_expression(:IF) do
    bodynode = self.body.class == Node::NEWLINE ? self.body.next : self.body
    elsenode = self.else.class == Node::NEWLINE ? self.else.next : self.else
    return "(#{self.cond.as_expression}) ? " +
           "(#{bodynode.as_expression}) : " +
           "(#{elsenode.as_expression})"
  end

  define_expression(:TRUENODE) do
    return "true"
  end

  define_expression(:FALSENODE) do
    return "false"
  end

  define_expression(:NILNODE) do
    return "nil"
  end

  define_expression(:DOT2) do
    return "(#{self.beg.as_expression})..(#{self.end.as_expression})"
  end

  define_expression(:DOT3) do
    return "(#{self.beg.as_expression})..(#{self.end.as_expression})"
  end

  define_expression(:GVAR) do
    return "#{self.vid}"
  end

  define_expression(:IVAR) do
    return "#{self.vid}"
  end

  define_expression(:CVAR) do
    return "#{self.vid}"
  end

  define_expression(:DVAR) do
    return "#{self.vid}"
  end

  define_expression(:NODE_NTH_REF) do
    return "$#{self.nth}"
  end

  define_expression(:DASGN_CURR) do
    return "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:DASGN) do
    return "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:LASGN) do
    return "#{self.vid} = #{self.value.as_expression}"
  end

  define_expression(:ATTR_ASGN) do
    case self.mid
    when :[]=
      args = self.args.to_a
      attrs = args[1..-2].map { |n| n.as_expression }
      value = args[-1].as_expression
      return "(#{self.recv.as_expression})[#{attrs.join(', ')}] = #{value}"
    else
      return "(#{self.recv.as_expression}).#{self.mid}#{self.args.as_expression(false)}"
    end
  end

  define_expression(:CONST) do
    return "#{self.vid}"
  end

  define_expression(:COLON2) do
    return "#{self.mid}::#{self.value.as_expression}"
  end

  define_expression(:LVAR) do
    return "#{self.vid}"
  end

  define_expression(:NEWLINE) do
    return self.next.as_expression
  end

  define_expression(:STR) do
    return "\"#{self.lit.inspect[1..-2]}\""
  end

  define_expression(:REGX) do
    # TODO: cflag
    return "/#{self.lit.inspect[1..-2]}/"
  end

  define_expression(:REGX_ONCE) do
    # TODO: cflag
    return "/#{self.lit.inspect[1..-2]}/o"
  end

  define_expression(:XSTR) do
    return "`#{self.lit.inspect[1..-2]}`"
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
    return s
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
    return s
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
    return s
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
    return s
  end

  major = Config::CONFIG['MAJOR'].to_i
  minor = Config::CONFIG['MINOR'].to_i
  teeny = Config::CONFIG['TEENY'].to_i
  ruby_version_code = major * 100 + minor * 10 + teeny

  if ruby_version_code >= 180 then

    define_expression(:EVSTR) do
      return "\#\{#{self.body.as_expression}\}"
    end

  else

    # TODO: not sure how to implement EVSTR on 1.6.x

  end

  define_expression(:ITER) do
    return "#{self.iter.as_expression} { #{self.body.as_expression} }"
  end

  define_expression(:WHILE) do
    if self.state == 1 then
      return "while #{self.cond.as_expression} do; #{self.body.as_expression}; end"
    else
      return "begin; #{self.body.as_expression}; end while #{self.cond.as_expression}"
    end
  end

  define_expression(:UNTIL) do
    if self.state == 1 then
      return "until #{self.cond.as_expression} do; #{self.body.as_expression}; end"
    else
      return "begin; #{self.body.as_expression}; end until #{self.cond.as_expression}"
    end
  end

  define_expression(:BREAK) do
    s = "break"
    if self.stts then
      s += " #{self.stts.as_expression}"
    end
    return s
  end

  define_expression(:RETURN) do
    s = "return"
    if self.stts then
      s += " #{self.stts.as_expression}"
    end
    return s
  end

  define_expression(:BEGIN) do
    return "begin; #{self.body.as_expression}; end"
  end

  define_expression(:ENSURE) do
    return "#{self.head.as_expression} ensure #{self.ensr.as_expression}"
  end

  define_expression(:RESCUE) do
    return "#{self.head.as_expression} rescue #{self.resq.as_expression}"
  end

  define_expression(:RESBODY) do
    if self.ensr then
      a = self.ensr.to_a.map { |n| n.as_expression }
      return "#{a.join(', ')}; #{self.resq.as_expression}"
    else
      return "#{self.resq.as_expression}"
    end
  end
end

