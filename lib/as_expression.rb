class Node
  class LIT < Node
    def as_expression
      return self.lit.to_s
    end
  end

  class FCALL < Node
    def as_expression
      args = self.args
      return "#{self.mid}(#{args ? args.as_expression(false) : ''})"
    end
  end

  @@arithmetic_expressions = [
    :+, :-, :*, :/, :<, :>, :<=, :>=, :==, :===,
    '!='.intern, '~='.intern, ':!~'.intern
  ]

  class CALL < Node
    def as_expression
      case self.mid
      when *@@arithmetic_expressions
        args = self.args
        return "#{self.recv.as_expression} #{self.mid} #{args ?  args.as_expression(false) : ''}"
      else
        args = self.args
        return "#{self.recv.as_expression}.#{self.mid}(#{args ?  args.as_expression(false) : ''})"
      end
    end
  end

  class VCALL < Node
    def as_expression
      return self.mid.to_s
    end
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

    def as_expression(brackets=true)
      s = brackets ? '[' : ''
      s += self.to_a.map { |n| n.as_expression }.join(', ')
      s += brackets ? ']' : ''
      return s
    end
  end

  class HASH < Node
    def as_expression
      a = self.head.to_a
      elems = []
      i = 0
      while i < a.size do
        elems << "#{a[i].as_expression}=>#{a[i+1].as_expression}"
        i += 2
      end
      return "{#{elems.join(', ')}}"
    end
  end

  class IF < Node
    def as_expression
      return "#{self.cond.as_expression} ? " +
             "#{self.body.as_expression} : " +
             "#{self.else.as_expression}"
    end
  end

  class TRUENODE < Node
    def as_expression
      return "true"
    end
  end

  class FALSENODE < Node
    def as_expression
      return "false"
    end
  end

  class NILNODE < Node
    def as_expression
      return "nil"
    end
  end

  def as_expression
    # default
    return self.class
    # return "<expression>"
  end
end

