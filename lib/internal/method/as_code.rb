require 'internal/node/as_code'
require 'internal/vm/iseq/as_code'
require 'internal/method'
require 'internal/method/signature'

module MethodAsCode
  # Returns a string representation of the method definition/body.
  #
  #   irb(main):001:0> def foo(a, b, *rest, &block)
  #   irb(main):002:1>   begin
  #   irb(main):003:2*     if not a and not b then
  #   irb(main):004:3*       raise "Need more input!"
  #   irb(main):005:3>     end
  #   irb(main):006:2>     return a + b
  #   irb(main):007:2>   ensure
  #   irb(main):008:2*     puts "In ensure block"
  #   irb(main):009:2>   end
  #   irb(main):010:1> end
  #   => nil
  #   irb(main):011:0> m = method(:foo)
  #   => #<Method: Object#foo>
  #   irb(main):012:0> puts m.as_code
  #   def foo(a, b, *rest, &block)
  #     begin
  #       (raise("Need more input!")) if (not a and not b)
  #       return a + b
  #     ensure
  #       puts("In ensure block")
  #     end
  #   end
  #
  def as_code(indent=0, name=nil)
    sig = self.signature
    if self.body.respond_to?(:body) then
      # YARV
      body_expression = self.body.body.as_code(indent+1)
    else
      # pre-YARV
      body_expression = self.body ? self.body.as_code(indent+1) : ''
    end
    name ||= sig.name
    s = "#{'  '*indent}def #{name}(#{sig.param_list})\n"
    if body_expression then
      s += "#{body_expression}\n"
    end
    s += "#{'  '*indent}end"
    return s
  end
end

class Method
  include MethodAsCode
end

class UnboundMethod
  include MethodAsCode
end

