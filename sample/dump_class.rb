require 'internal/node'

class Foo
  def foo; puts "this is a test..."; end
end

s = Marshal.dump(Foo)
p Marshal.load(s) #=> Foo

