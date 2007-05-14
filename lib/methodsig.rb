require 'nodewrap'
require 'as_expression'

module MethodSig
  # Return the names of the arguments this method takes, in the order in
  # which they appear in the argument list.
  def argument_names
    if self.body.respond_to?(:tbl) then
      # pre-YARV
      return self.body.tbl || []
    else
      # YARV
      iseq = self.body.body
      local_vars = iseq.local_table
      has_rest_arg = iseq.arg_rest != -1
      has_block_arg = iseq.arg_block != -1
      num_args = \
        iseq.argc + \
        iseq.arg_opt_table.size + \
        (has_rest_arg ? 1 : 0) + \
        (has_block_arg ? 1 : 0)
      return local_vars[0...num_args]
    end
  end

  def args_node
    if self.body.respond_to?(:next)
      # pre-YARV
      if self.body.next.class == Node::ARGS then
        return self.body.next
      elsif self.body.next.head.class == Node::ARGS then
        return self.body.next.head
      else
        raise "Could not find method arguments"
      end
    else
      # YARV
      return nil
    end
  end
  private :args_node

  # If this method has a "rest" argument, that is, it has an argument
  # that is preceded by an asterisk (*) in the argument list, then
  # return its index, otherwise return nil.
  def rest_arg
    args_node = args_node()
    if args_node then
      # pre-YARV
      rest = args_node.rest()
      if rest.class == Node::LASGN then
        # subtract 2 to account for implicit vars
        return rest.cnt - 2
      elsif not rest
        return nil
      else
        return rest > 0 ? rest - 2 : nil
      end
    else
      # YARV
      rest = self.body.body.arg_rest
      return rest >= 0 ? rest - 1: nil
    end
  end

  # If this method has a "block" argument, that is, it has an argument
  # that is preceded by an ampersand (&) in the argument list, then
  # return its index, otherwise return nil.
  def block_arg
    if self.body.respond_to?(:next) then
      # pre-YARV
      block = self.body.next
      if block.class == Node::BLOCK and
         block.next.head.class == Node::BLOCK_ARG then
        # subtract 2 to account for implicit vars
        return block.next.head.cnt - 2
      else
        return nil
      end
    else
      # YARV
      arg_block = self.body.body.arg_block
      return arg_block >= 0 ? arg_block - 1 : nil
    end
  end

  # Return a hash mapping each argument name to a description of that
  # argument.
  def argument_info
    names = self.argument_names()
    block_arg = self.block_arg()

    info = {}
    names.each do |name|
      info[name] = name.to_s
    end

    # Optional args
    args_node = args_node()
    if args_node then
      # pre-YARV
      opt = args_node().opt
      while opt do
        head = opt.head
        if head.class == Node::LASGN then
          info[head.vid] = "#{head.vid}=#{head.value.as_expression}"
        else
          raise "Unexpected node type: #{opt.class}"
        end
        opt = opt.next
      end
    else
      # YARV
      opt = self.body.body.arg_opt_table.size
    end

    # Rest arg
    if self.rest_arg then
      rest_name = names[rest_arg]
      info[rest_name] = "*#{rest_name}"
    end

    # Block arg
    if block_arg then
      block_name = names[block_arg]
      info[block_name] = "&#{block_name}"
    end

    return info
  end

  # An abstraction for a method signature.
  class Signature
    attr_reader :origin_class, :name, :arg_names, :arg_info

    def initialize(origin_class, name, arg_names, arg_info)
      @origin_class = origin_class
      @name = name
      @arg_names = arg_names
      @arg_info = arg_info
    end

    def to_s
      params = @arg_names.map{ |n| arg_info[n] }
      return "#{@origin_class}\##{@name}(#{param_list})"
    end

    def param_list
      params = @arg_names.map{ |n| arg_info[n] }
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
        argument_info)
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
        argument_info)
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

