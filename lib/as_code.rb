require 'as_code'

class Node
  public

  def as_code(indent=0, *args)
    return as_code_impl(self, indent, *args)
  end

  private

  def as_code_impl(node, indent, *args)
    # default -- most code is just an expression
    "#{'  '*indent}#{node.as_expression(*args)}"
  end

  class << self
    def define_code(klass, &block)
      if const_defined?(klass) then
        const_get(klass).instance_eval { define_method(:as_code_impl, &block) }
      end
    end
  end

  define_code(:IF) do |node, indent|
    if node.body.class == Node::BLOCK or
       node.else.class == Node::BLOCK then
      s = "#{'  '*indent}if #{node.cond.as_expression} then\n"
      s << "#{'  '*indent}#{node.body.as_code(indent+1)}\n"
      if node.else then
        s << "#{'  '*indent}else\n"
        s << "#{'  '*indent}#{node.else.as_code(indent+1)}\n"
      end
      s << "#{'  '*indent}end"
      s
    else
      node.as_expression
    end
  end

  define_code(:BLOCK) do |node, indent|
    a = node.to_a
    if a[0].class == Node::DASGN_CURR and not a[0].value
      # ignore variable definitions
      a.shift
    end
    a.map { |n| "#{n.as_code(indent)}" }.join("\n")
  end

  define_code(:ITER) do |node, indent|
    "#{'  '*indent}#{node.iter.as_expression} {\n" +
    "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
    "#{'  '*indent}}"
  end

  define_code(:WHILE) do |node, indent|
    if node.state == 1 then
      "#{'  '*indent}while #{node.cond.as_expression} do\n" +
      "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end"
    else
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end while #{node.cond.as_expression}"
    end
  end

  define_code(:UNTIL) do |node, indent|
    if node.state == 1 then
      "#{'  '*indent}until #{node.cond.as_expression} do\n" +
      "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end"
    else
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end until #{node.cond.as_expression}"
    end
  end

  define_code(:BEGIN) do |node, indent|
    if node.body.class == Node::RESCUE then
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}#{node.body.as_code(indent+1, true)}\n" +
      "#{'  '*indent}end"
    elsif node.body then
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
      "#{'  '*indent}end"
    else
      "#{'  '*indent}begin\n" +
      "#{'  '*indent}end\n"
    end
  end

  define_code(:ENSURE) do |node, indent|
    s = ''
    if node.head then
      s << "#{'  '*indent}#{node.head.as_code(indent+1)}\n"
    end
    s << "#{'  '*indent}ensure\n" +
    s << "#{'  '*indent}#{node.ensr.as_code(indent+1)}"
    s
  end

  define_code(:RESCUE) do |node, indent, *args|
    begin_rescue = args[0] || false
    if node.head then
      if begin_rescue then
        "#{'  '*indent}#{node.head.as_code(indent+1)}\n" +
        "#{'  '*indent}rescue #{node.resq.as_code(indent+1, begin_rescue)}"
      else
        "#{node.head.as_expression} rescue #{node.resq.as_expression(begin_rescue)}"
      end
    else
      "rescue #{node.resq.as_expression(begin_rescue)}"
    end
  end

  define_code(:RESBODY) do |node, indent, *args|
    begin_rescue = args[0] || false
    if begin_rescue then
      if node.ensr then
        a = node.ensr.to_a.map { |n| n.as_expression }
        "#{a.join(', ')}\n" +
        "#{'  '*indent}#{node.resq.as_code(indent+1)}"
      else
        node.resq ? "\n#{'  '*indent}#{node.resq.as_code(indent+1)}" : ''
      end
    else
      # TODO: assuming node.ensr is false...
      node.resq ? node.resq.as_code : ''
    end
  end

  define_code(:NEWLINE) do |node, indent|
    node.next.as_code(indent)
  end

  define_code(:CASE) do |node, indent|
    "#{'  '*indent}case #{node.head.as_expression}\n" +
    "#{node.body.as_code(indent)}end"
  end

  define_code(:WHEN) do |node, indent|
    args = node.head.to_a.map { |n| n.as_expression }
    s = "#{'  '*indent}when #{args.join(', ')}\n"
    if node.body then
      s << "#{'  '*indent}#{node.body.as_code(indent+1)}; "
    end
    if node.next then
      s << node.next.as_code
    end
    s
  end

  define_code(:CLASS) do |node, indent|
    s_super = node.super ? " < #{node.super.as_expression}" : ''
    "#{'  '*indent}class #{node.cpath.as_expression}#{s_super}\n" +
    "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_code(:SCLASS) do |node, indent|
    "#{'  '*indent}class << #{node.recv.as_expression}\n" +
    "#{'  '*indent}#{node.body.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_code(:DEFN) do |node, indent|
    # TODO: what to do about noex?
    "#{'  '*indent}def #{node.mid}\n" +
    "#{'  '*indent}#{node.next.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end

  define_code(:DEFS) do |node, indent|
    "#{'  '*indent}def #{node.recv.as_expression}.#{node.mid}\n" +
    "#{'  '*indent}#{node.next.as_code(indent+1)}\n" +
    "#{'  '*indent}end"
  end
end

