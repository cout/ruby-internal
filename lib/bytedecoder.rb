require 'nodewrap'

module Nodewrap

# A module for decoding YARV bytecode.
#
# This is actually pretty cool.  It's actually a miniature VM, where the
# result of evaluating an expression is itself another expression.  This
# turns out to be much simpler than a full ruby VM, but I think one
# could use this as a base for building one.
#
# Example usage:
#   env = Nodewrap::ByteDecoder::Environment.new(is.local_table)
#   is = VM::InstructionSequence.new('1 + 1')
#   is.bytedecode(env)
#   env.expressions.each do |expr|
#     puts expr
#   end
#   puts stack[-1]
#
module ByteDecoder

class Environment
  attr_reader :stack
  attr_reader :expressions
  attr_reader :local_table
  attr_accessor :last

  def initialize(local_table)
    @stack = []
    @expressions = []
    @local_table = local_table
    @last = nil
  end
end

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

  class Literal < Expression
    attr_reader :value

    def initialize(value)
      @value = value
    end

    def to_s
      case @value
      when Regexp then "/#{@value.inspect[1..-2]}/"
      else; return @value.inspect
      end
    end

    def precedence
      return 1
    end
  end

  class Infix < Expression
    attr_reader :op
    attr_reader :lhs
    attr_reader :rhs

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
      when :*, :/, :%
        return 2
      when :+, :-
        return 3
      when :<<, :>>
        return 4
      when :>, :>=, :<, :<=, :==, :===
        return 5
      else
        raise ArgumentError, "Unknown op: #{@op}"
      end
    end
  end

  class Prefix < Expression
    def initialize(op, expr)
      @op = op
      @expr = expr
    end

    def to_s
      op = @op.to_s
      op.chop! if op[-1] == ?@
      if @op == :"!" and @expr.is_a?(Infix) and @expr.op == :== then
        return "#{@expr.fmt(@expr.lhs)} != #{@expr.fmt(@expr.rhs)}"
      elsif self.precedence < @expr.precedence then
        return "#{op}(#{@expr})"
      else
        return "#{op}#{@expr}"
      end
    end

    def precedence
      return 1
    end
  end

  class Send < Expression
    attr_reader :is_assignment

    def initialize(id, has_receiver, has_parens, receiver, block, splat_last, *args)
      @id = id
      @is_assignment = id.to_s[-1] == ?=
      @has_receiver = has_receiver
      @has_parens = has_parens
      @receiver = receiver
      @block = block
      @splat_last = splat_last
      @args = args
    end

    def to_s
      s = ''
      receiver_str = @has_receiver \
        ? "#{@receiver}." \
        : nil
      args = @args.map { |x| x.to_s }
      if @splat_last then
        args[-1] = "*#{@args[-1]}"
      end
      if @is_assignment and args.size == 1 then
        s = "#{receiver_str}#{@id.to_s[0..-2]} = #{args[0]}"
      else
        open = @has_parens ? '(' : ''
        close = @has_parens ? ')' : ''
        s = "#{receiver_str}#{@id}#{open}#{args.join(', ')}#{close}"
      end
      if @block then
        env = Environment.new(@block.local_table)
        @block.bytedecode(env)
        expressions = (env.expressions + env.stack).map { |x| x.to_s }
        s << " { #{expressions.join('; ')} }"
      end
      return s
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

  class Constant < Expression
    def initialize(klass, name)
      @klass = klass
      @name = name
    end

    def to_s
      if @klass then
        if @klass == Object then
          return "::#{@name}"
        else
          return "#{@klass}::#{@name}"
        end
      else
        return "#{@name}"
      end
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
        when Literal
          case arg.value
          when String then s << arg.value
          else; s << arg.to_s
          end
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
        p @value, string, unstring
        return Regexp.compile(unstring).inspect
      else
        return Regexp.compile(@value.to_s).inspect
      end
    end

    def precedence
      return 1
    end
  end

  class Throw < Expression
    def initialize(value)
      @value = value
    end

    def to_s
      # TODO: not all throws are breaks...
      if @value then
        return "break #{@value}"
      else
        return "break"
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
      def bytedecode(env)
        env.stack.push Expression::Literal.new(self.operands[0])
      end
    end

    INFIX_OPCODES = {
      OPT_PLUS  => :+,
      OPT_MINUS => :-,
      OPT_MULT  => :*,
      OPT_DIV   => :/,
      OPT_MOD   => :%,
      OPT_LTLT  => :>>,
      # OPT_GTGT  => :<<,
      OPT_EQ    => :==,
      OPT_GT    => :>,
      OPT_GE    => :>=,
      OPT_LT    => :<,
      OPT_LE    => :<=,
    }

    INFIX_OPERATORS = INFIX_OPCODES.values + [ :===, :<< ]

    INFIX_OPCODES.each do |klass, op|
      klass.class_eval do
        define_method(:bytedecode) do |env|
          rhs = env.stack.pop
          lhs = env.stack.pop
          env.stack.push Expression::Infix.new(op, lhs, rhs)
        end
      end
    end

    PREFIX_OPCODES = {
      PUTNOT => :"!",
    }

    PREFIX_OPERATORS = PREFIX_OPCODES.values + [ :~, :+@, :-@ ]

    PREFIX_OPCODES.each do |klass, op|
      klass.class_eval do
        define_method(:bytedecode) do |env|
          expr = env.stack.pop
          env.stack.push Expression::Prefix.new(op, expr)
        end
      end
    end

    class TRACE
      def bytedecode(env)
      end
    end

    class LEAVE
      def bytedecode(env)
      end
    end

    LITERAL_OPCODES = [
      PUTNIL,
      DUPARRAY,
      PUTSTRING,
    ]

    LITERAL_OPCODES.each do |klass|
      klass.class_eval do
        define_method(:bytedecode) do |env|
          env.stack.push Expression::Literal.new(@operands[0])
        end
      end
    end

    class SEND
      def bytedecode(env)
        id = @operands[0]
        num_args = @operands[1]
        args = []
        num_args.times do
          args.unshift env.stack.pop
        end
        has_receiver = !flag_set(VM::CALL_FCALL_BIT)
        has_parens = !flag_set(VM::CALL_VCALL_BIT)
        splat_last = flag_set(VM::CALL_ARGS_SPLAT_BIT)
        receiver = env.stack.pop
        block = @operands[2]
        if INFIX_OPERATORS.include?(id) and args.size == 1 then
          env.stack.push Expression::Infix.new(id, receiver, args[0])
        elsif PREFIX_OPERATORS.include?(id) and args.size == 0 then
          env.stack.push Expression::Prefix.new(id, receiver)
        else
          env.stack.push Expression::Send.new(
              id, has_receiver, has_parens, receiver, block, splat_last, *args)
        end
      end

      def flag_set(flag)
        flags = @operands[3]
        return flags & flag == flag
      end
    end

    class PUTSELF
      def bytedecode(env)
        env.stack.push Expression::Self.new
      end
    end

    class NEWHASH
      def bytedecode(env)
        i = @operands[0]
        args = []
        while i > 0 do
          args.unshift env.stack.pop
          i -= 1
        end
        env.stack.push Expression::Hash.new(args)
      end
    end

    class NEWARRAY
      def bytedecode(env)
        i = @operands[0]
        args = []
        while i > 0 do
          args.unshift env.stack.pop
          i -= 1
        end
        env.stack.push Expression::Array.new(args)
      end
    end

    class DEFINED
      def bytedecode(env)
        env.stack.push Expression::Defined.new(@operands[1])
      end
    end

    GET_VARIABLE_OPCODES = [
      GETCLASSVARIABLE,
      GETINSTANCEVARIABLE,
      GETGLOBAL,
    ]

    GET_VARIABLE_OPCODES.each do |klass|
      klass.class_eval do
        define_method(:bytedecode) do |env|
          env.stack.push Expression::Variable.new(@operands[0])
        end
      end
    end

    SET_VARIABLE_OPCODES = [
      SETCLASSVARIABLE,
      SETINSTANCEVARIABLE,
      SETGLOBAL,
    ]

    SET_VARIABLE_OPCODES.each do |klass|
      klass.class_eval do
        define_method(:bytedecode) do |env|
          value = env.stack.pop
          env.stack.delete_at(-1) # TODO: dup'd value.. is this right?
          env.stack.push Expression::Assignment.new(@operands[0], value)
        end
      end
    end

    class GETCONSTANT
      def bytedecode(env)
        klass = env.stack.pop
        env.stack.push Expression::Constant.new(klass, @operands[0])
      end
    end

    class GETSPECIAL
      def bytedecode(env)
        type = @operands[1] >> 1
        env.stack.push Expression::Variable.new("$#{type.chr}")
      end
    end

    class GETINLINECACHE
      def bytedecode(env)
        env.stack.push nil
      end
    end

    class SETINLINECACHE
      def bytedecode(env)
      end
    end

    class NOP
      def bytedecode(env)
      end
    end

    class TOSTRING
      def bytedecode(env)
      end
    end

    class CONCATSTRINGS
      def bytedecode(env)
        i = @operands[0]
        args = []
        while i > 0 do
          args.unshift env.stack.pop
          i -= 1
        end
        env.stack.push Expression::ConcatStrings.new(args)
      end
    end

    class TOREGEXP
      def bytedecode(env)
        env.stack.push Expression::ToRegexp.new(env.stack.pop)
      end
    end

    class DUP
      def bytedecode(env)
        arg = env.stack.pop
        env.stack.push arg
        env.stack.push arg
      end
    end

    class SETLOCAL
      def bytedecode(env)
        idx = env.local_table.size - @operands[0] + 1
        name = env.local_table[idx]
        value = env.stack.pop
        env.stack.push Expression::Assignment.new(name, value)
      end
    end

    class GETLOCAL
      def bytedecode(env)
        idx = env.local_table.size - @operands[0] + 1
        name = env.local_table[idx]
        env.stack.push Expression::Variable.new(name)
      end
    end

    class SETDYNAMIC
      def bytedecode(env)
        idx = env.local_table.size - @operands[0] + 1
        name = env.local_table[-idx]
        value = env.stack.pop
        env.stack.push Expression::Assignment.new(name, value)
      end
    end

    class GETDYNAMIC
      def bytedecode(env)
        idx = env.local_table.size - @operands[0] + 1
        name = env.local_table[-idx]
        env.stack.push Expression::Variable.new(name)
      end
    end

    class SETN
      # set nth stack entry to stack top
      def bytedecode(env)
        n = @operands[0]
        dest = -(n+1)
        if env.stack[dest].is_a?(Expression) then
          env.expressions.push env.stack[dest]
        end
        env.stack[dest] = env.stack[-1]
      end
    end

    class POP
      def bytedecode(env)
        top = env.stack[-1]
        if top.is_a?(Expression) then
          if top.is_a?(Expression::Send) and top.is_assignment then
            # special case - the return value from the assignment gets
            # thrown away and the result is the rhs
            env.stack.delete_at(-2)
          end
          env.expressions.push top
        end
        env.stack.pop
      end
    end

    class THROW
      def bytedecode(env)
        value = env.stack.pop
        env.expressions.push env.stack.pop
        env.stack.push Expression::Throw.new(value)
      end
    end
  end

  class InstructionSequence
    def bytedecode(env)
      self.each do |instruction|
        # p instruction
        instruction.bytedecode(env)
      end
    end
  end
end

if __FILE__ == $0 then
  def foo; @@foo = 1; end
  # def foo; a = [2, 3]; foo(1, *a); end
  # def foo; not true; end
  # def foo; catch(:foo) { throw :foo; 42 }; end
  # def foo; a ? b : c; end
  # def foo; loop { a = 1; break }; end
  # def foo; ::BAR; end
  # def foo; a != b; end
  # def foo; 1 - 2; end
  # def foo; +a; end
  # def foo; !a; end
  # def foo; a << b; end
  # def foo; a === b; end
  # def foo; []; end
  # def foo; foo.bar = 42; end
  # def foo; h = {}; h.default = true; h; end
  # def foo; @FOO; end
  # def foo; $FOO; end
  # def foo; /foo#{bar}/; end
  # def foo; /foo/; end
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

  env = Nodewrap::ByteDecoder::Environment.new(is.local_table)
  s = ''
  # puts "local_table = #{is.local_table.inspect}"
  is.each do |i|
    # p i.operand_types, i.operand_names
    # p i #, i.operand_types, i.operand_names
    print i.class, ' '
    a = []
    i.operand_names.each_with_index do |name, idx|
      a << "#{name}(#{i.operand_types[idx]})=#{i.operands[idx].inspect}"
    end
    puts a.join(', ')
    i.bytedecode(env)
    # p env.stack
    # p env.stack.map { |x| x.to_s }
    # puts
  end

  (env.expressions + env.stack).each do |expr|
    puts expr.to_s
  end
end

