module MethodOrigin
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

  # Return a Method::Origin representing where the method was defined.
  def origin
    block = body().next
    return Origin.new(block.nd_file, block.nd_line)
  end
end

class Method
  include MethodOrigin
end

class UnboundMethod
  include MethodOrigin
end
