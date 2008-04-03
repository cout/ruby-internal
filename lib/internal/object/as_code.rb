require 'internal/node/as_code'

class Object
  def as_code(indent=0)
    # TODO: this won't work for many objects
    "#{'  '*indent}#{self.inspect}"
  end
end

