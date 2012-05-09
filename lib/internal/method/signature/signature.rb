module MethodSig
  # An abstraction for a method signature.
  class Signature
    attr_reader :origin_class, :name, :arg_names, :args

    def initialize(origin_class, name, arg_names, args)
      @origin_class = origin_class
      @name = name
      @arg_names = arg_names
      @args = args
    end

    def to_s
      return "#{@origin_class}\##{@name}(#{param_list})"
    end

    def param_list
      params = @arg_names.map{ |n| args[n].to_s }
      return params.join(', ')
    end
  end
end

