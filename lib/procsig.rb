require 'node_to_a'

class Proc
  class Arguments
    include Enumerable

    def initialize(names, multiple_assignment, rest_arg)
      @names = names
      @multiple_assignment = multiple_assignment
      @rest_arg = rest_arg
    end

    def unspecified
      if defined?(VM) and defined?(VM::InstructionSequence) then
        # YARV
        return @names.length == 0
      else
        # pre-YARV
        @names.nil?
      end
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

    def empty_last_arg
      if defined?(VM) and defined?(VM::InstructionSequence) then
        # YARV
        return (@rest_arg and @names[-1] == nil)
      else
        # pre-YARV
        return (@names.size == 1 and not @rest_arg)
      end
    end
  end

  # Return an Arguments object representing the arguments in the order
  # in which they appear in the argument list.
  def arguments
    has_rest_arg = self.has_rest_arg

    if self.respond_to?(:var) then
      # pre-YARV
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
      when Fixnum, nil # TODO: ?
        return Arguments.new([], false, has_rest_arg ? 0 : nil)
      else
        raise "Unexpected node type: #{self.var.class}"
      end
    elsif
      # YARV
      iseq = self.body
      local_vars = iseq.local_table
      has_rest_arg = iseq.arg_rest != -1
      has_block_arg = iseq.arg_block != -1
      num_args = \
        iseq.argc + \
        iseq.arg_opt_table.size + \
        (has_rest_arg ? 1 : 0) + \
        (has_block_arg ? 1 : 0)
      puts "argc=#{iseq.argc}, opt_table.size=#{iseq.arg_opt_table.size}"
      puts "has_rest_arg=#{has_rest_arg}, has_block_arg=#{has_block_arg}"
      names = local_vars[0...num_args]
      # TODO: masgn
      return Arguments.new(names, true, has_rest_arg ? -1 : nil)
    else
      return Arguments.new(nil, false, nil)
    end
  end

  # Return true if the proc has a rest arg
  def has_rest_arg
    if self.respond_to?(:var) then
      # pre-YARV
      has_rest_arg = false
      if self.var then
        if self.var.class == Node::MASGN then
          if self.var.args then
            has_rest_arg = true
          end
        end
      end
    else
      # YARV
      rest = self.body.arg_rest
      has_rest_arg = (rest >= 0 ? rest - 1 : nil)
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
      if rest_name then
        info[rest_name] = "*#{rest_name}"
      end
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
        if @args.empty_last_arg then
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

