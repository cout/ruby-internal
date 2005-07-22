require 'as_code'

class Node
  def as_code(indent=0)
    # default -- most code is just an expression
    self.as_code_impl(indent)
  end

  def as_code_impl(indent)
    # default -- most code is just an expression
    "#{'  '*indent}#{self.as_expression}"
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
      "#{'  '*indent}if #{self.cond.as_expression} then\n" +
      "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
      "#{'  '*indent}else\n" +
      "#{'  '*indent}#{self.else.as_code(indent+1)}\n" +
      "#{'  '*indent}end"
    else
      self.as_expression
    end
  end

  define_code(:BLOCK) do |indent|
    a = self.to_a
    if a[0].class == Node::DASGN_CURR and not a[0].value
      # ignore variable definitions
      a.shift
    end
    a.map { |n| "#{n.as_code(indent)}" }.join("\n")
  end

  define_code(:ITER) do |indent|
    "#{'  '*indent}#{self.iter.as_expression} {\n" +
    "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
    "#{'  '*indent}}"
  end

  define_code(:WHILE) do |indent|
    if self.state == 1 then
      "#{'  '*indent}while #{self.cond.as_expression} do\n" +
      "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end"
    else
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end while #{self.cond.as_expression}"
    end
  end

  define_code(:UNTIL) do |indent|
    if self.state == 1 then
      "#{'  '*indent}until #{self.cond.as_expression} do\n" +
      "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end"
    else
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end until #{self.cond.as_expression}"
    end
  end

  define_expression(:BEGIN) do |indent|
    "#{'  '*indent}begin\n" +
    "#{'  '*indent}#{self.bdoy.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_expression(:ENSURE) do
    "#{'  '*indent}#{self.head.as_code(indent+1)}\n" +
    "#{'  '*indent}ensure\n" +
    "#{'  '*indent}#{self.ensr.as_code(indent+1)}"
  end

  define_expression(:RESCUE) do
    "#{'  '*indent}#{self.head.as_code(indent+1)}\n" +
    "#{'  '*indent}rescue #{self.resq.as_code(indent+1)}"
  end

  define_expression(:RESBODY) do
    if self.ensr then
      a = self.ensr.to_a.map { |n| n.as_expression }
      "#{a.join(', ')}\n" +
      "#{'  '*indent}#{self.resq.as_code(indent+1)}"
    else
      "#{self.resq.as_code(indent+1)}"
    end
  end

  define_code(:NEWLINE) do |indent|
    self.next.as_code(indent)
  end

  define_code(:CASE) do
    "#{'  '*indent}case #{self.head.as_expression}\n" +
    "#{self.body.as_code(indent)}end"
  end

  define_code(:WHEN) do
    args = self.head.to_a.map { |n| n.as_expression }
    "#{'  '*indent}when #{args.join(', ')}\n" +
    "#{'  '*indent}#{self.body.as_code(indent+1)}; "
  end

  define_expression(:CLASS) do
    s_super = self.super ? " < #{self.super.as_expression}" : ''
    "#{'  '*indent}class #{self.cpath.as_expression}#{s_super}\n" +
    "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_expression(:SCLASS) do
    "#{'  '*indent}class << #{self.recv.as_expression}\n" +
    "#{'  '*indent}#{self.body.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_expression(:DEFN) do
    # TODO: what to do about noex?
    "#{'  '*indent}def #{self.mid}\n" +
    "#{'  '*indent}#{self.next.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_expression(:DEFS) do
    "#{'  '*indent}def #{self.recv.as_expression}.#{self.mid}\n" +
    "#{'  '*indent}#{self.next.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end
end

