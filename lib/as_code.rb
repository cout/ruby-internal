require 'nodewrap'
require 'rbconfig'
require 'as_expression'

if defined?(VM::InstructionSequence) then
  require 'bytedecoder'

  class VM
    class InstructionSequence
      def as_code(indent=0)
        env = Nodewrap::ByteDecoder::Environment.new(local_table())
        opt_pc = self.opt_pc
        self.bytedecode(env, opt_pc)
        expressions = env.expressions + env.stack
        if expressions.length == 0 then
          return nil
        elsif expressions.length == 1 and
           expressions[0].is_a?(Nodewrap::ByteDecoder::Expression::Literal) and
           expressions[0].value == nil then
          return nil
        else
          expressions.map! { |e| "#{'  '*indent}#{e}" }
          return expressions.join("\n")
        end
      end
    end
  end
end

class Node
  public

  def as_code(indent=0, *args)
    return as_code_impl(self, indent, *args)
  end

  private

  def as_code_impl(node, indent, *args)
    # default -- most code is just an expression
    expression = node.as_expression(*args)
    if not expression.nil? then
      return "#{'  '*indent}#{expression}"
    else
      return nil
    end
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
      "#{'  '*indent}#{node.as_expression}"
    end
  end

  define_code(:BLOCK) do |node, indent|
    a = node.to_a
    if a[0].class == Node::DASGN_CURR and not a[0].value
      # ignore variable definitions
      a.shift
    end
    lines = a.map { |n| n.as_code(indent) }
    lines.reject! { |e| e.nil? }
    if lines.size == 0 then
      "#{'  '*indent}nil"
    else
      lines.join("\n")
    end
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
      s = "#{'  '*indent}begin\n" +
      "#{node.body.as_code(indent+1, true)}\n" +
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
    s = ''
    if not have_begin then
      s << "#{'  '*indent}begin\n"
      indent += 1
    end
    yield s, indent, true
    if not have_begin then
      indent -= 1
      s << "\n#{'  '*indent}end"
    end
    return s
  end

  define_code(:ENSURE) do |node, indent, *args|
    begin_ensure = args[0] || false
    s = ''
    Node.begin_end(indent, begin_ensure) do |s, indent, begin_ensure|
      if node.head then
        s << "#{node.head.as_code(indent)}\n"
      end
      s << "#{'  '*(indent-1)}ensure\n"
      s << "#{node.ensr.as_code(indent)}"
    end
  end

  define_code(:RESCUE) do |node, indent, *args|
    begin_rescue = args[0] || false
    Node.begin_end(indent, begin_rescue) do |s, indent, begin_rescue|
      if node.head then
        if begin_rescue then
          s << "#{node.head.as_code(indent)}\n"
          s << "#{'  '*(indent-1)}rescue #{node.resq.as_code(indent+1, begin_rescue)}"
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
    if node.respond_to?(:cpath) then
      path = node.cpath.as_expression
    else
      path = node.cname
    end
    "#{'  '*indent}class #{path}#{s_super}\n" +
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

  define_code(:SCOPE) do |node, indent|
    case node.next
    when nil then ''
    when Node::ARGS then "#{'  '*indent}nil"
    when Node::BLOCK_ARG then "#{'  '*indent}nil"
    else node.next.as_code(indent)
    end
  end
end

module MethodAsCode
  def as_code(indent=0, name=nil)
    sig = self.signature
    body = self.body ? self.body.as_code(indent+1) : ''
    name ||= sig.name
    s = "#{'  '*indent}def #{name}(#{sig.param_list})\n"
    if self.body then
      s += "#{self.body.as_code(indent+1)}\n"
    end
    s += "#{'  '*indent}end"
    return s
  end
end

class Method
  include MethodAsCode
end

class UnboundMethod
  include MethodAsCode
end

class Proc
  def as_code(indent=0)
    sig = self.signature
    body = self.body ? self.body.as_code(indent+1) : ''
    s = "#{'  '*indent}proc do"
    if not sig.args.unspecified then
      s += " #{sig}"
    end
    s += "\n"
    if self.body then
      s += "#{self.body.as_code(indent+1)}\n"
    end
    s += "#{'  '*indent}end"
    return s
  end
end

class Module
  # TODO: it would be nice if we could go back and find the AST
  # for the class instead of recreating the code from the class's
  # current state.
  def as_code(indent=0)
    imethods = self.instance_methods - self.superclass.instance_methods
    cmethods = self.instance_methods - self.superclass.instance_methods
    constants = self.constants - self.superclass.constants
    name = self.name.gsub(/.*::/, '')

    # TODO: included modules?
    if self.class == Class then
      s = "#{'  '*indent}class #{name} < #{self.superclass}\n"
    else
      s = "#{'  '*indent}module #{name}\n"
    end

    constants.each do |constant|
      s += "#{'  '*indent} #{constant}=#{self.const_get(constant).as_code}\n"
    end

    # TODO: protected/private
    imethods.each do |method|
      s += self.instance_method(method).as_code(indent+1)
      s += "\n"
    end

    cmethods.each do |method|
      s += self.instance_method(method).as_code(indent+1, "self.#{method}")
      s += "\n"
    end

    # TODO: singleton class constants
    # TODO: class variables
    # TODO: singleton instance variables

    s += "#{'  '*indent}end"

    return s
  end
end

class Object
  def as_code(indent=0)
    # TODO: this won't work for many objects
    "#{'  '*indent}#{self.inspect}"
  end
end

