class Node
  class ARRAY
    def to_a
      a = []
      node = self
      while node do
        a.push(node.head)
        node = node.next
      end
    end
  end
end

class Proc
  class Arguments
    include Enumerable

    def initialize(names, multiple_assignment, rest_arg)
      @names = names
      @multiple_assignment = multiple_assignment
      @rest_arg = rest_arg
    end

    def unspecified
      @names.nil?
    end

    def single_assignment
      !@multiple_assignment
    end

    def multiple_assignment
      @multiple_assignment
    end

    def names
      @names
    end

    def [](idx)
      @names[idx]
    end

    def each(&block)
      (@names || []).each(&block)
    end

    def size
      @names.size
    end

    def rest_arg
      @rest_arg
    end
  end

  # Return an Arguments object representing the arguments in the order
  # in which they appear in the argument list.
  def arguments
    has_rest_arg = self.has_rest_arg

    if self.var then
      case self.var
      when Node::DASGN_CURR
        return Arguments.new([ self.var.vid ], false, has_rest_arg ? 0 : nil)
      when Node::MASGN
        if self.var.head then
          a = self.var.head.to_a
          args = a.map { |n| n.vid }
        else
          args = []
        end
        if self.var.args then
          args.push(self.var.args.vid)
        end
        return Arguments.new(args, true, has_rest_arg ? args.size - 1: nil)
      when Fixnum
        return Arguments.new([], false, has_rest_arg ? 0 : nil)
      else
        raise "Unexpected node type: #{self.var.class}"
      end
    else
      return Arguments.new(nil, false, nil)
    end
  end

  # Return true if the proc has a rest arg
  def has_rest_arg
    has_rest_arg = false
    if self.var then
      if self.var.class == Node::MASGN then
        if self.var.args then
          has_rest_arg = true
        end
      end
    end
    return has_rest_arg
  end

  # Return a hash mapping each argument name to a description of that
  # argument.
  def argument_info
    args = self.arguments()

    info = {}
    args.each do |name|
      info[name] = name.to_s
    end

    # Rest arg
    if args.rest_arg then
      rest_name = args[args.rest_arg]
      info[rest_name] = "*#{rest_name}"
    end

    return info
  end

  class Signature
    attr_reader :args, :arg_info

    def initialize(args, arg_info)
      @args = args
      @arg_info = arg_info
    end

    def to_s
      if @args.unspecified then
        return ""
      elsif @args.multiple_assignment then
        if @args.size == 1 and not @args.rest_arg then
          params = @args.map{ |n| arg_info[n] }
          return "|#{@arg_info[@args[0]]},|"
        else
          params = @args.map{ |n| arg_info[n] }
          return "|#{params.join(', ')}|"
        end
      else
        return "|#{@arg_info[@args[0]]}|"
      end
    end
  end

  # Return a String representing the method's signature.
  def signature
    return Signature.new(
        arguments(),
        argument_info)
  end
end

