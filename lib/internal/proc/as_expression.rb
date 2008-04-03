require 'internal/node/as_expression'
require 'internal/proc/signature'

class Proc
  def as_expression
    sig = self.signature
    body_expression = self.body ? self.body.as_expression : nil
    s = sig.args.unspecified ? "" : sig.to_s + ' '
    b = body_expression ? body_expression + ' ' : ''
    return "proc { #{s}#{b}}"
  end
end

