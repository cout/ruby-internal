require 'internal/method'
require 'internal/node'
require 'internal/node/as_expression'
require 'internal/method/signature/argument'
require 'internal/method/signature/iseq'
require 'internal/method/signature/node'
require 'internal/method/signature/signature'

if defined?(RubyVM) then
require 'internal/vm/bytedecoder'
end

module MethodSig
  # Return the names of the local variables of this method.
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
      args[name] = Argument.new(name, false, false)
    end

    # Optional args
    args_node = args_node()
    set_optional_args(args, args_node, names)

    # Rest arg
    if self.rest_arg then
      rest_name = names[rest_arg]
      args[rest_name] = Argument.new(rest_name, true, false)
    end

    # Block arg
    if block_arg then
      block_name = names[block_arg]
      args[block_name] = Argument.new(block_name, false, true)
    end

    return args
  end
end

class Method
  include MethodSig

  # Return a String representing the method's signature.
  def signature
    return Signature.new(
        attached_class(),
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
        origin_class(),
        method_oid().to_s,
        argument_names(),
        arguments())
  end
end

if __FILE__ == $0 then
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

