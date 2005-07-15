require 'as_code'

class Node
  def as_code(indent=0)
    # default -- most code is just an expression
    return self.as_code_impl(indent)
  end

  def as_code_impl(indent)
    # default -- most code is just an expression
    return "#{'  '*indent}#{self.as_expression}"
  end

  class << self
    def define_code(klass, &block)
      if const_defined?(klass) then
        const_get(klass).__send__(:define_method, :as_code_impl, &block)
      end
    end
  end

  define_code(:IF) do |indent|
    if self.body.class == Node::BLOCK or
       self.else.class == Node::BLOCK then
      return "#{'  '*indent}if #{self.cond.as_expression} then\n" +
             "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
             "#{'  '*indent}else\n" +
             "#{'  '*indent}#{self.else.as_code(indent+1)}\n" +
             "#{'  '*indent}end"
    else
      return self.as_expression
    end
  end

  define_code(:BLOCK) do |indent|
    a = self.to_a
    if a[0].class == Node::DASGN_CURR then
      # don't do anything for variable definitions
      a.shift
    end
    return a.map { |n| "#{n.as_code(indent)}" }.join("\n")
  end

  define_code(:ITER) do |indent|
    return "#{'  '*indent}#{self.iter.as_expression} {\n" +
           "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
           "#{'  '*indent}}"
  end

  define_code(:NEWLINE) do |indent|
    return self.next.as_code(indent)
  end
end

