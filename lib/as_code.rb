require 'nodewrap'
require 'rbconfig'

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
    if node.body.class == Node::RESCUE or
       node.body.class == Node::ENSURE then
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

  def self.begin_end(indent, have_begin)
    major = Config::CONFIG['MAJOR'].to_i
    minor = Config::CONFIG['MINOR'].to_i
    teeny = Config::CONFIG['TEENY'].to_i
    ruby_version_code = major * 100 + minor * 10 + teeny
    s = ''
    if ruby_version_code >= 190 then
      s << "#{'  '*indent}begin\n"
      indent += 1
      have_begin = true
    end
    yield s, indent + 1, have_begin
    if ruby_version_code >= 190 then
      indent -= 1
      s << "\n#{'  '*indent}end"
    end
    return s
  end

  define_code(:ENSURE) do |node, indent, *args|
    begin_ensure = args[0] || false
    Node.begin_end(indent, begin_ensure) do |s, indent, begin_ensure|
      if node.head then
        s << "#{'  '*indent}#{node.head.as_code(indent+1)}\n"
      end
      s << "#{'  '*indent}ensure\n"
      s << "#{'  '*indent}#{node.ensr.as_code(indent+1)}"
    end
  end

  define_code(:RESCUE) do |node, indent, *args|
    begin_rescue = args[0] || false
    Node.begin_end(indent, begin_rescue) do |s, indent, begin_rescue|
      if node.head then
        if begin_rescue then
          s << "#{'  '*indent}#{node.head.as_code(indent+1)}\n"
          s << "#{'  '*indent}rescue #{node.resq.as_code(indent+1, begin_rescue)}"
        else
          s << "#{node.head.as_expression} rescue #{node.resq.as_expression(begin_rescue)}"
        end
      else
        s << "rescue #{node.resq.as_expression(begin_rescue)}"
      end
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

