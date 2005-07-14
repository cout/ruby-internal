require 'as_expression'

class Node
  def as_code
    # default -- most code is just an expression
    return self.as_expression
  end

  class << self
    def define_code(klass, &block)
      if const_defined?(klass) then
        const_get(klass).__send__(:define_method, :as_code, &block)
      end
    end
  end

  define_code(:IF) do
    if self.body.class == Node::BLOCK or
       self.else.class == Node::BLOCK then
      return "if #{self.cond.as_code} then\n" +
             "#{self.body.as_code}" +
             "else\n" +
             "#{self.else.as_code}" +
             "end"
    else
      bodynode = self.body.class == Node::NEWLINE ? self.body.next : self.body
      elsenode = self.else.class == Node::NEWLINE ? self.else.next : self.else
      return "#{self.cond.as_code} ? " +
             "#{bodynode.as_code} : " +
             "#{elsenode.as_code}"
    end
  end

  define_code(:NEWLINE) do
    return "#{self.next.as_code}\n"
  end

  define_code(:BLOCK) do
    a = self.to_a
    if a[0].class == Node::DASGN_CURR then
      # don't do anything for variable definitions
      a.shift
    end
    return a.map { |n| n.as_expression }.join
  end
end

