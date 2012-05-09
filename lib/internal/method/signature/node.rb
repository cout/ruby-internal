class Node
  # pre-YARV
  class SCOPE
    include MethodSig

    def local_vars
      return self.tbl || []
    end

    def argument_names
      local_vars = self.local_vars
      args = self.args_node
      num_required_args = args.cnt
      num_optional_args = 0
      opt = args.opt
      while opt do
        num_optional_args += 1
        opt = opt.next
      end
      num_args = \
        num_required_args + \
        num_optional_args + \
        (rest_arg ? 1 : 0) + \
        (block_arg ? 1 : 0)
      return local_vars[0...num_args]
    end

    def args_node
      if self.next.class == Node::ARGS then
        return self.next
      elsif self.next.head.class == Node::ARGS then
        return self.next.head
      else
        raise "Could not find method arguments"
      end
    end

    def rest_arg
      args_node = args_node()
      rest = args_node.rest()
      if rest.class == Node::LASGN then
        # subtract 2 to account for implicit vars
        return rest.cnt - 2
      elsif not rest
        return nil
      else
        return rest > 0 ? rest - 2 : nil
      end
    end

    def block_arg
      block = self.next
      if block.class == Node::BLOCK and
         block.next.head.class == Node::BLOCK_ARG then
        # subtract 2 to account for implicit vars
        return block.next.head.cnt - 2
      else
        return nil
      end
    end

    def set_optional_args(args, args_node, names)
      opt = args_node.opt
      while opt do
        head = opt.head
        if head.class == Node::LASGN then
          args[head.vid] = NodeOptionalArgument.new(
              head.vid, head.value.as_expression, head.value, false, false)
        else
          raise "Unexpected node type: #{opt.class}"
        end
        opt = opt.next
      end
    end
  end

  # Pre-YARV, I think
  class BMETHOD
    def local_vars
      raise "TODO: Not implemented yet"
    end

    def argument_names
      return self.cval.arguments.names
    end

    def arguments
      return self.cval.arguments
    end

    def args_node
      raise "TODO: Not implemented yet"
    end

    def rest_arg
      raise "TODO: Not implemented yet"
    end

    def block_arg
      raise "TODO: Not implemented yet"
    end

    def set_optional_args(args, args_node, names)
      raise "TODO: Not implemented yet"
    end
  end

  # YARV up to 1.9.1
  class METHOD
    include MethodSig

    def local_vars
      iseq = self.body
      local_vars = iseq.local_table
      return local_vars
    end

    def argument_names
      local_vars = self.local_vars
      iseq = self.body
      opt_args = iseq.arg_opt_table
      opt_args.pop # last arg is a pointer to the start of the code
      num_args = \
        iseq.argc + \
        opt_args.size + \
        (rest_arg ? 1 : 0) + \
        (block_arg ? 1 : 0)
      return local_vars[0...num_args]
    end

    def args_node
      return nil
    end

    def rest_arg
      arg_rest = self.body.arg_rest
      return arg_rest >= 0 ? arg_rest : nil
    end

    def block_arg
      arg_block = self.body.arg_block
      return arg_block >= 0 ? arg_block : nil
    end

    def set_optional_args(args, args_node, names)
      opt_table = self.body.arg_opt_table
      opt_table.pop
      first_opt_idx =
        names.size -
        opt_table.size -
        (self.rest_arg ? 1 : 0) -
        (self.block_arg ? 1 : 0)
      opt_table.each_with_index do |pc, idx|
        name = names[first_opt_idx + idx]
        args[name] = YarvOptionalArgument.new(name, self.body, pc, idx, false, false)
      end
    end
  end
end

