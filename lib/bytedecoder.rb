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
    def initialize(id, has_receiver, has_parens, receiver, *args)
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
      open_paren = @has_parens ? '(' : ''
      close_paren = @has_parens ? ')' : ''
      return "#{receiver_str}#{@id}#{open_paren}#{args.join(', ')}#{close_paren}"
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

  class Hash < Expression
    def initialize(args)
      @args = args
    end

    def to_s
      s = '{ '
      a = []
      i = 0
      while i < @args.length do
        a << "#{@args[i]} => #{@args[i + 1]}"
        i += 2
      end
      s << a.join(', ')
      s << ' }'
      return s
    end

    def precedence
      return 1
    end
  end

  class Array < Expression
    def initialize(args)
      @args = args
    end

    def to_s
      s = '[ '
      s << @args.join(', ')
      s << ' ]'
      return s
    end

    def precedence
      return 1
    end
  end

  class Defined < Expression
    def initialize(arg)
      @arg = arg
    end

    def to_s
      return "defined?(#{@arg.to_s})"
    end

    def precedence
      return 1
    end
  end

  class Variable < Expression
    def initialize(name)
      @name = name
    end

    def to_s
      return @name.to_s
    end

    def precedence
      return 1
    end
  end

  class ConcatStrings < Expression
    def initialize(args)
      @args = args
    end

    def to_s
      s = "\""
      @args.each do |arg|
        case arg
        when String
          s << arg
        else
          s << "\#{#{arg.to_s}}"
        end
      end
      s << "\""
    end

    def precedence
      return 1
    end
  end

  class Assignment < Expression
    def initialize(name, value)
      @name = name
      @value = value
    end

    def to_s
      return "#{@name} = #{@value}"
    end

    def precedence
      return 1
    end
  end

  class ToRegexp < Expression
    def initialize(value)
      @value = value
    end

    def to_s
      case @value
      when ConcatStrings
        string = @value.to_s
        unstring = string[1..-2]
        return Regexp.compile(unstring).inspect
      else
        return Regexp.compile(@value.to_s).inspect
      end
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
      def push_expression(stack, local_table)
        stack.push self.operands[0]
      end
    end

    INFIX_OPCODES = {
      OPT_PLUS => :+,
      OPT_MULT => :*,
      OPT_DIV  => :/,
      OPT_EQ   => :==,
      OPT_GT   => :>,
      OPT_GE   => :>=,
      OPT_LT   => :<,
      OPT_LE   => :<=,
    }

    INFIX_OPERATORS = INFIX_OPCODES.values

    INFIX_OPCODES.each do |klass, op|
      klass.class_eval do
        define_method(:push_expression) do |stack, local_table|
          rhs = stack.pop
          lhs = stack.pop
          stack.push Expression::Infix.new(op, lhs, rhs)
        end
      end
    end

    class TRACE
      def push_expression(stack, local_table)
      end
    end

    class LEAVE
      def push_expression(stack, local_table)
      end
    end

    LITERAL_OPCODES = [
      PUTNIL,
      DUPARRAY,
      PUTSTRING,
    ]

    LITERAL_OPCODES.each do |klass|
      klass.class_eval do
        define_method(:push_expression) do |stack, local_table|
          stack.push @operands[0]
        end
      end
    end

    class SEND
      def push_expression(stack, local_table)
        id = @operands[0]
        num_args = @operands[1]
        args = []
        num_args.times do
          args.unshift stack.pop
        end
        has_receiver = flag_set(VM::CALL_FCALL_BIT)
        has_parens = !flag_set(VM::CALL_VCALL_BIT)
        receiver = stack.pop
        if INFIX_OPERATORS.include?(id) then
          stack.push Expression::Infix.new(id, args[0], args[1])
        else
          stack.push Expression::Send.new(
              id, has_receiver, has_parens, receiver, *args)
        end
      end

      def flag_set(flag)
        flags = @operands[3]
        return flags & flag != flag
      end
    end

    class PUTSELF
      def push_expression(stack, local_table)
        stack.push Expression::Self.new
      end
    end

    class NEWHASH
      def push_expression(stack, local_table)
        i = @operands[0]
        args = []
        while i > 0 do
          args.unshift stack.pop
          i -= 1
        end
        stack.push Expression::Hash.new(args)
      end
    end

    class NEWARRAY
      def push_expression(stack, local_table)
        i = @operands[0]
        args = []
        while i > 0 do
          args.unshift stack.pop
          i -= 1
        end
        stack.push Expression::Array.new(args)
      end
    end

    class DEFINED
      def push_expression(stack, local_table)
        stack.push Expression::Defined.new(@operands[1])
      end
    end

    VARIABLE_OPCODES = [
      GETCLASSVARIABLE,
      GETINSTANCEVARIABLE,
      GETCONSTANT,
      GETGLOBAL,
    ]

    VARIABLE_OPCODES.each do |klass|
      klass.class_eval do
        define_method(:push_expression) do |stack, local_table|
          stack.push Expression::Variable.new(@operands[0])
        end
      end
    end

    class GETSPECIAL
      def push_expression(stack, local_table)
        type = @operands[1] >> 1
        stack.push Expression::Variable.new("$#{type.chr}")
      end
    end

    class GETINLINECACHE
      def push_expression(stack, local_table)
      end
    end

    class SETINLINECACHE
      def push_expression(stack, local_table)
      end
    end

    class NOP
      def push_expression(stack, local_table)
      end
    end

    class TOSTRING
      def push_expression(stack, local_table)
      end
    end

    class CONCATSTRINGS
      def push_expression(stack, local_table)
        i = @operands[0]
        args = []
        while i > 0 do
          args.unshift stack.pop
          i -= 1
        end
        stack.push Expression::ConcatStrings.new(args)
      end
    end

    class TOREGEXP
      def push_expression(stack, local_table)
        stack.push Expression::ToRegexp.new(stack.pop)
      end
    end

    class DUP
      def push_expression(stack, local_table)
        arg = stack.pop
        stack.push arg
        stack.push arg
      end
    end

    class SETLOCAL
      # TODO: SETLOCAL doesn't really push anything back onto the stack,
      # afaict -- so I'm not really sure what to do with the result.
      # Maybe I need to keep track of certain expressions off the stack,
      # somehow.  I don't know.
      def push_expression(stack, local_table)
        name = local_table[local_table.size - @operands[0] + 1]
        value = stack.pop
        stack.push Expression::Assignment.new(name, value)
      end
    end

    class GETLOCAL
      def push_expression(stack, local_table)
        name = local_table[local_table.size - @operands[0] + 1]
        stack.push Expression::Variable.new(name)
      end
    end

    class SETN
      # set nth stack entry to stack top
      def push_expression(stack, local_table)
        n = @operands[0]
        p stack
        stack[-n] = stack.pop
        p stack
      end
    end

    class POP
      def push_expression(stack, local_table)
        stack.pop
      end
    end
  end
end

if __FILE__ == $0 then
  # def foo; foo.bar = 42; end
  def foo; h = {}; h.default = true; h; end
  # def foo; @FOO; end
  # def foo; $FOO; end
  # def foo; /foo#{bar}/; end
  # def foo; foo = 1; bar=2; baz=3 end
  # def foo; "foo#{bar}"; end
  # def foo; $`; end
  # def foo; FOO; end
  # def foo; @@foo; end
  # def foo; defined?(FOO); end
  # def foo; [1+1, 2, 3, 4] ; end
  # def foo; { 1 => 2, 3 => 4 }; end
  # def foo; foo(1 + 2 * (3 + 4), 5); end
  # def foo; 1 + 2 * (3 + 4); end

  n = method(:foo).body
  is = n.body
  puts is.disasm

  stack = []
  s = ''
  # puts "local_table = #{is.local_table.inspect}"
  is.each do |i|
    # p i.operand_types, i.operand_names
    p i #, i.operand_types, i.operand_names
    i.push_expression(stack, is.local_table)
    # p stack
  end

  stack.each do |expr|
    puts expr.to_s
  end
end

