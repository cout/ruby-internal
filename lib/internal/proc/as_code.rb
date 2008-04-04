require 'internal/node/as_code'
require 'internal/proc'
require 'internal/proc/signature'

class Proc
  def as_code(indent=0)
    sig = self.signature
    body_expression = self.body ? self.body.as_code(indent+1) : nil
    s = "#{'  '*indent}proc do"
    if not sig.args.unspecified then
      s += " #{sig}"
    end
    s += "\n"
    if body_expression then
      s += "#{body_expression}\n"
    end
    s += "#{'  '*indent}end"
    return s
  end
end

