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
    :^, :%, '!'.intern, '!='.intern, '~='.intern, ':!~'.intern
  ]

  define_expression(:CALL) do
    case self.mid
    when *@@arithmetic_expressions
      args = self.args
      return "#{self.recv.as_expression} #{self.mid} #{args ? args.as_expression(false) : ''}"
    else
      args = self.args
      return "#{self.recv.as_expression}.#{self.mid}(#{args ? args.as_expression(false) : ''})"
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

    def as_expression(brackets = false)
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

    def as_expression(brackets = false)
      return brackets ? '[]' : ''
    end
  end

  define_expression(:HASH) do
    a = self.head.to_a
    elems = []
    i = 0
    while i < a.size do
      elems << "#{a[i].as_expression}=>#{a[i+1].as_expression}"
      i += 2
    end
    return "{#{elems.join(', ')}}"
  end

  define_expression(:IF) do
    return "#{self.cond.as_expression} ? " +
           "#{self.body.as_expression} : " +
           "#{self.else.as_expression}"
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
    return "#{self.beg.as_expression}..#{self.end.as_expression}"
  end

  define_expression(:DOT3) do
    return "#{self.beg.as_expression}..#{self.end.as_expression}"
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

  define_expression(:NODE_NTH_REF) do
    return "$#{self.nth}"
  end

  define_expression(:ATTR_ASGN) do
    case self.mid
    when :[]=
      args = self.args.to_a
      attrs = args[1..-2].map { |n| n.as_expression }
      value = args[-1].as_expression
      return "#{self.recv.as_expression}[#{attrs.join(', ')}] = #{value}"
    else
      return "#{self.recv.as_expression}.#{self.mid}#{self.args.as_expression(false)}"
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
    return "#{self.next.as_expression}\n"
  end
end

