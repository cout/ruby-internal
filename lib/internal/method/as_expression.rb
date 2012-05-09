require 'internal/node/as_expression'
require 'internal/vm/iseq/as_expression'
require 'internal/method/signature'
require 'internal/method'

module MethodAsExpression
  # Return a single-line string representation of a method
  # TODO: this method would be more aptly named "as_expression_string".
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

