require 'internal/node/as_code'
require 'internal/vm/iseq/as_code'
require 'internal/method'
require 'internal/method/signature'

module MethodAsCode
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

