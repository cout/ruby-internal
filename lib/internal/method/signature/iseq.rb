class RubyVM
  # YARV 1.9.2 and later
  class InstructionSequence
    include MethodSig

    def local_vars
      local_vars = self.local_table
      return local_vars
    end

    def argument_names
      local_vars = self.local_vars
      opt_args = self.arg_opt_table
      opt_args.pop # last arg is a pointer to the start of the code
      num_args = \
        self.argc + \
        opt_args.size + \
        (rest_arg ? 1 : 0) + \
        (block_arg ? 1 : 0)
      return local_vars[0...num_args]
    end

    def args_node
      return nil
    end

    def rest_arg
      arg_rest = self.arg_rest
      return arg_rest >= 0 ? arg_rest : nil
    end

    def block_arg
      arg_block = self.arg_block
      return arg_block >= 0 ? arg_block : nil
    end

    def set_optional_args(args, args_node, names)
      opt_table = self.arg_opt_table
      opt_table.pop
      first_opt_idx =
        names.size -
        opt_table.size -
        (self.rest_arg ? 1 : 0) -
        (self.block_arg ? 1 : 0)
      opt_table.each_with_index do |pc, idx|
        name = names[first_opt_idx + idx]
        args[name] = YarvOptionalArgument.new(name, self, pc, idx, false, false)
      end
    end
  end
end

