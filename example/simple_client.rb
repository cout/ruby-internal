require 'nodewrap'

n = Marshal.load($stdin)

Foo = Class.new(Object)
Foo.instance_eval do
  add_method(:foo, n, 0)
end

f = Foo.new
f.foo

