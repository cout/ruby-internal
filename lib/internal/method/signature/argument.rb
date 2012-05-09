
module MethodSig
  class Argument
    attr_reader :name

    def required?
      return !optional?
    end

    def optional?
      return rest? || block?
    end

    def rest?
      return @is_rest
    end

    def block?
      return @is_block
    end

    def default
      return nil
    end

    def initialize(name, is_rest, is_block)
      @name = name
      @is_rest = is_rest
      @is_block = is_block
    end

    def to_s
      if @is_rest then
        prefix = "*"
      elsif @is_block then
        prefix = "&"
      end

      if self.default then
        suffix = "=#{default()}"
      end

      return "#{prefix}#{@name}#{suffix}"
    end
  end

  class OptionalArgument < Argument
    def optional?
      return true
    end
  end

  class NodeOptionalArgument < OptionalArgument
    attr_reader :node_for_default
    attr_reader :default

    def initialize(name, default, node_for_default, is_rest, is_block)
      super(name, is_rest, is_block)
      @default = default
      @node_for_default = node_for_default
    end
  end

  class YarvOptionalArgument < OptionalArgument
    attr_reader :iseq
    attr_reader :pc_start
    attr_reader :local_idx

    def initialize(name, iseq, pc_start, local_idx, is_rest, is_block)
      super(name, is_rest, is_block)
      @iseq = iseq
      @pc_start = pc_start
      @local_idx = local_idx
      @default = nil
    end

    def inspect
      default()
      super
    end

    def default
      return @default if @default

      env = Internal::ByteDecoder::Environment.new(@iseq.local_table())
      local_table_idx = local_table_idx()
      @iseq.bytedecode(env, @pc_start) do |instr|
        RubyVM::Instruction::SETLOCAL === instr &&
        instr.operands[0] == local_table_idx
      end
      expressions = env.expressions + env.stack

      @default = expressions[0].rhs.to_s
      return @default
    end

    def local_table_idx
      return @iseq.local_table.size - @local_idx + 1
    end
  end
end

