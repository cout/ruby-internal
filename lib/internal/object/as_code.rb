require 'internal/node/as_code'
require 'internal/object'

class Object
  def as_code(indent=0)
    # TODO: this won't work for many objects
    "#{'  '*indent}#{self.inspect}"
  end
end

