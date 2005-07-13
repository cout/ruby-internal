require 'nodewrap'

class Method
  # Return the names of the arguments this method takes, in the order in
  # which they appear in the argument list.
  def argument_names
    return body().tbl || []
  end

  # Return true if this method has a "rest" argument, that is, it has
  # an argument that is preceded by an asterisk (*) in the argument
  # list.
  def has_rest_arg
    block = body().next
    return block.head.args.rest > 0
  end

  # Return true if this method has a "block" argument, that is, it has
  # an argument that is preceded by an ampersand (&) in the argument
  # list.
  def has_block_arg
    block = body().next
    return block.next.head.class == Node::BLOCK_ARG
  end

  # Return a hash mapping each argument name to a description of that
  # argument.
  def argument_info
    names = argument_names()
    block = body().next
    args = block.head

    info = {}
    names.each do |name|
      info[name] = name.to_s
    end

    # Optional args
    opt = args.opt
    while opt do
      if opt.head.class == Node::LASGN then
        info[opt.head.vid] = "#{opt.head.vid}=<expression>"
      else
        raise "Unexpected node type: #{opt.class}"
      end
      opt = opt.next
    end

    # Rest arg
    if args.rest > 0 then
      rest_name = names[args.rest - 2] # skip $_ and $~
      info[rest_name] = "*#{rest_name}"
    end

    # Block arg
    if block.next.head.class == Node::BLOCK_ARG then
      block_name = names[block.next.head.cnt - 2] # skip $_ and $~
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
      return "#{@origin_class}\##{@name}(#{params.join(', ')})"
    end
  end

  # Return a String representing the method's signature.
  def signature
    return Signature.new(
        origin_class() || attached_class(),
        method_oid().to_s,
        argument_names(),
        argument_info)
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

  # Return a MethodOrigin object representing where the method was
  # defined.
  def origin
    block = body().next
    return Origin.new(block.nd_file, block.nd_line)
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
end

