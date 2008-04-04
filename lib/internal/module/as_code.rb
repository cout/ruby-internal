require 'internal/node/as_code'
require 'internal/module'

class Module
  # TODO: it would be nice if we could go back and find the AST
  # for the class instead of recreating the code from the class's
  # current state.
  def as_code(indent=0)
    imethods = self.instance_methods - self.superclass.instance_methods
    cmethods = self.instance_methods - self.superclass.instance_methods
    constants = self.constants - self.superclass.constants
    name = self.name.gsub(/.*::/, '')

    # TODO: included modules?
    if self.class == Class then
      s = "#{'  '*indent}class #{name} < #{self.superclass}\n"
    else
      s = "#{'  '*indent}module #{name}\n"
    end

    constants.each do |constant|
      s += "#{'  '*indent} #{constant}=#{self.const_get(constant).as_code}\n"
    end

    # TODO: protected/private
    imethods.each do |method|
      s += self.instance_method(method).as_code(indent+1)
      s += "\n"
    end

    cmethods.each do |method|
      s += self.instance_method(method).as_code(indent+1, "self.#{method}")
      s += "\n"
    end

    # TODO: singleton class constants
    # TODO: class variables
    # TODO: singleton instance variables

    s += "#{'  '*indent}end"

    return s
  end
end

