require 'nodewrap'
require 'as_expression'

module MethodSig
  class Argument
    attr_reader :name
    attr_reader :default
    attr_reader :node_or_iseq_for_default

    def optional?
      return (@default != nil) || @is_rest || @is_block
    end

    def required?
      return !optional?
    end

    def rest?
      return @is_rest
    end

    def block?
      return @is_block
    end

    def initialize(name, default, node_or_iseq_for_default, is_rest, is_block)
      @name = name
      @default = default
      @node_or_iseq_for_default = node_or_iseq_for_default
      @is_rest = is_rest
      @is_block = is_block
    end

    def to_s
      if @is_rest then
        prefix = "*"
      elsif @is_block then
        prefix = "&"
      end

      if @default then
        suffix = "=#{@default}"
      end

      return "#{prefix}#{@name}#{suffix}"
    end
  end

  def local_vars
    return self.body.local_vars
  end

  # Return the names of the arguments this method takes, in the order in
  # which they appear in the argument list.
  def argument_names
    return self.body.argument_names
  end

  def args_node
    return self.body.args_node
  end
  private :args_node

  # If this method has a "rest" argument, that is, it has an argument
  # that is preceded by an asterisk (*) in the argument list, then
  # return its index, otherwise return nil.
  def rest_arg
    return self.body.rest_arg
  end

  # If this method has a "block" argument, that is, it has an argument
  # that is preceded by an ampersand (&) in the argument list, then
  # return its index, otherwise return nil.
  def block_arg
    return self.body.block_arg
  end

  def set_optional_args(args, args_node, names)
    self.body.set_optional_args(args, args_node, names)
  end
  private :set_optional_args

  # Return a hash mapping each argument name to a description of that
  # argument.
  def arguments
    names = self.argument_names()
    block_arg = self.block_arg()

    args = {}
    names.each do |name|
      args[name] = Argument.new(name, nil, nil, false, false)
    end

    # Optional args
    args_node = args_node()
    set_optional_args(args, args_node, names)

    # Rest arg
    if self.rest_arg then
      rest_name = names[rest_arg]
      args[rest_name] = Argument.new(rest_name, nil, nil, true, false)
    end

    # Block arg
    if block_arg then
      block_name = names[block_arg]
      args[block_name] = Argument.new(block_name, nil, nil, false, true)
    end

    return args
  end

  # An abstraction for a method signature.
  class Signature
    attr_reader :origin_class, :name, :arg_names, :args

    def initialize(origin_class, name, arg_names, args)
      @origin_class = origin_class
      @name = name
      @arg_names = arg_names
      @args = args
    end

    def to_s
      return "#{@origin_class}\##{@name}(#{param_list})"
    end

    def param_list
      params = @arg_names.map{ |n| args[n].to_s }
      return params.join(', ')
    end
  end

  # An abstraction for a method origin.
  class Origin
    attr_reader :file, :line

    def initialize(file, line)
      @file = file
      @line = line
    end

    def to_s
      return "#{file}:#{line}"
    end
  end

  # Return a Method::Origin representing where the method was defined.
  def origin
    block = body().next
    return Origin.new(block.nd_file, block.nd_line)
  end
end

class Method
  include MethodSig

  # Return a String representing the method's signature.
  def signature
    return Signature.new(
        origin_class() || attached_class(),
        method_oid().to_s,
        argument_names(),
        arguments())
  end
end

class UnboundMethod
  include MethodSig

  # Return a String representing the method's signature.
  def signature
    return Signature.new(
        nil,
        method_oid().to_s,
        argument_names(),
        arguments())
  end
end

class Node
  # pre-YARV
  class SCOPE
    include MethodSig

    def local_vars
      return self.tbl || []
    end

    def argument_names
      local_vars = self.local_vars
      num_required_args = self.next.head.cnt
      num_optional_args = 0
      opt = self.next.head.opt
      while opt do
        num_optional_args += 1
        opt = opt.next
      end
      num_args = \
        num_required_args + \
        num_optional_args + \
        (rest_arg ? 1 : 0) + \
        (block_arg ? 1 : 0)
      return local_vars[0...num_args]
    end

    def args_node
      if self.next.class == Node::ARGS then
        return self.body.next
      elsif self.next.head.class == Node::ARGS then
        return self.next.head
      else
        raise "Could not find method arguments"
      end
    end

    def rest_arg
      args_node = args_node()
      rest = args_node.rest()
      if rest.class == Node::LASGN then
        # subtract 2 to account for implicit vars
        return rest.cnt - 2
      elsif not rest
        return nil
      else
        return rest > 0 ? rest - 2 : nil
      end
    end

    def block_arg
      block = self.next
      if block.class == Node::BLOCK and
         block.next.head.class == Node::BLOCK_ARG then
        # subtract 2 to account for implicit vars
        return block.next.head.cnt - 2
      else
        return nil
      end
    end

    def set_optional_args(args, args_node, names)
      opt = args_node.opt
      while opt do
        head = opt.head
        if head.class == Node::LASGN then
          args[head.vid] = Argument.new(head.vid, head.value.as_expression, head.value, false, false)
        else
          raise "Unexpected node type: #{opt.class}"
        end
        opt = opt.next
      end
    end
  end

  # YARV
  class METHOD
    include MethodSig

    def local_vars
      iseq = self.body
      local_vars = iseq.local_table
      return local_vars
    end

    def argument_names
      local_vars = self.local_vars
      iseq = self.body
      opt_args = iseq.arg_opt_table
      opt_args.pop # last arg is a pointer to the start of the code
      num_args = \
        iseq.argc + \
        opt_args.size + \
        (rest_arg ? 1 : 0) + \
        (block_arg ? 1 : 0)
      return local_vars[0...num_args]
    end

    def args_node
      return nil
    end

    def rest_arg
      rest = self.body.arg_rest
      return rest >= 0 ? rest - 1: nil
    end

    def block_arg
      arg_block = self.arg_block
      return arg_block >= 0 ? arg_block - 1 : nil
    end

    def set_optional_args(args, args_node, names)
      iseq = self.body
      opt_pc = iseq.opt_pc
      env = Nodewrap::ByteDecoder::Environment.new(iseq.local_table())
      iseq.bytedecode(env, 0, opt_pc)
      expressions = env.expressions + env.stack
      expressions.sort!
      opt_table = self.body.body.arg_opt_table
      opt_table.pop
      first_opt_idx =
        names.size -
        opt_table.size -
        (self.rest_arg ? 1 : 0) -
        (self.block_arg ? 1 : 0)
      opt_table.each_with_index do |pc, idx|
        name = names[first_opt_idx + idx]
        expr = expressions.find { |e| e.pc >= pc }
        args[name] = Argument.new(name, expr.rhs, nil, false, false) # TODO
      end
    end
  end
end

if __FILE__ == $0 then
  require 'nodewrap'

  def foo(); end
  puts method(:foo).signature

  def foo(foo); end
  puts method(:foo).signature

  def foo(foo, bar); end
  puts method(:foo).signature

  def foo(foo=42, bar=10); end
  puts method(:foo).signature

  def foo(*args); end
  puts method(:foo).signature

  def foo(foo, bar=42, *args, &block); end
  puts method(:foo).signature
  puts method(:foo).origin

  def foo(foo, bar=obj.foo(1, 2, foo(10)), *args, &block); end
  puts method(:foo).signature

  def foo(foo, bar=obj.foo(1 + 1), *args, &block); end
  puts method(:foo).signature

  def foo(foo, bar=true ? false : 0, *args, &block); end
  puts method(:foo).signature

  def foo(foo, bar=true, *args, &block); end
  puts method(:foo).signature

  def foo(foo, bar=nil, *args, &block); end
  puts method(:foo).signature

  def foo(foo, bar={1=>2}, *args, &block); end
  puts method(:foo).signature

  def foo(foo, bar=[1,2], *args, &block); end
  puts method(:foo).signature
end

