require 'internal/node/as_expression'
require 'internal/vm/iseq/as_expression'
require 'internal/method/signature'
require 'internal/method'

module MethodAsExpression
  # It doesn't entirely make sense to have Method#as_expression, because
  # a method definition isn't an expression.  We have one anyway, to be
  # consistent with Proc.
  def as_expression
    sig = self.signature
    if self.body.respond_to?(:body) then
      # YARV
      body_expression = self.body.body.as_expression
    else
      # pre-YARV
      body_expression = self.body.as_expression
    end
    if body_expression then
      return "def #{sig.name}(#{sig.param_list}); #{body_expression}; end"
    else
      return "def #{sig.name}(#{sig.param_list}); end"
    end
  end
end

class Method
  include MethodAsExpression
end

class UnboundMethod
  include MethodAsExpression
end

