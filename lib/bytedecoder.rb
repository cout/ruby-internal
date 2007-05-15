require 'nodewrap'

module Nodewrap

module ByteDecoder

class Expression
  def initialize
  end

  def fmt(arg)
    if arg.respond_to?(:precedence)
      p = arg.precedence
    else
      p = 0
    end

    if p >= self.precedence then
      return "(#{arg})"
    else
      return arg
    end
  end

  class Infix < Expression
    def initialize(op, lhs, rhs)
      @op = op
      @lhs = lhs
      @rhs = rhs
    end

    def to_s
      return "#{fmt(@lhs)} #{@op} #{fmt(@rhs)}"
    end

    def precedence
      case @op
      when :*, :/
        return 2
      when :+, :-
        return 3
      else
        raise ArgumentError, "Unknown op: #{@op}"
      end
    end
  end

  class Send < Expression
    def initialize(id, has_receiver, receiver, *args)
      @id = id
      @has_receiver = has_receiver
      @receiver = receiver
      @args = args
    end

    def to_s
      receiver_str = @has_receiver \
        ? "#{@receiver}." \
        : nil
      args = @args.map { |x| x.to_s }
      return "#{receiver_str}#{@id}(#{args.join(', ')})"
    end

    def precedence
      if @has_receiver then
        if @receiver.respond_to?(:precedence) then
          return @receiver.precedence
        end
      end
      return 1
    end
  end

  class Self < Expression
    def to_s
      return "self"
    end

    def precedence
      return 1
    end
  end
end

end # ByteDecoder

end # Nodewrap

class VM
  class Instruction
    include Nodewrap::ByteDecoder

    class PUTOBJECT
      def push_expression(stack)
        stack.push self.operands[0]
      end
    end

    class OPT_PLUS
      def push_expression(stack)
        rhs = stack.pop
        lhs = stack.pop
        stack.push Expression::Infix.new(:+, lhs, rhs)
      end
    end

    class OPT_MULT
      def push_expression(stack)
        rhs = stack.pop
        lhs = stack.pop
        stack.push Expression::Infix.new(:*, lhs, rhs)
      end
    end

    class TRACE
      def push_expression(stack)
      end
    end

    class LEAVE
      def push_expression(stack)
      end
    end

    class PUTNIL
      def push_expression(stack)
        stack.push nil
      end
    end

    class SEND
      def push_expression(stack)
        id = @operands[0]
        num_args = @operands[1]
        args = []
        num_args.times do
          args.unshift stack.pop
        end
        has_receiver = @operands[3] != 8 # TODO
        receiver = stack.pop
        stack.push Expression::Send.new(id, has_receiver, receiver, *args)
      end
    end

    class PUTSELF
      def push_expression(stack)
        stack.push Expression::Self.new
      end
    end
  end
end

if __FILE__ == $0 then
  def foo; foo(1 + 2 * (3 + 4), 5); end
  # def foo; 1 + 2 * (3 + 4); end

  n = method(:foo).body
  is = n.body
  puts is.disasm

  stack = []
  s = ''
  is.each do |i|
    p i #, i.operand_types, i.operand_names
    i.push_expression(stack)
    # p stack
  end

  p stack
  puts stack[-1].to_s
end

