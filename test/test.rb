require 'nodewrap'

def foo
  puts 'foo!'
  p self
end

m = Object.method(:foo)

$n = Node.method_node(m)
class Foo
  add_method(:foo, $n, 0)
end

p $n.nd_type
p $n.to_hsh
d = Marshal.dump($n);
# p d

$n2 = Marshal.load(d)
p $n2
p $n2.nd_type
p $n2.to_hsh # TODO: this causes a segfault

class Foo
  add_method(:foo, $n2, 0)
end
 
f = Foo.new
f.foo

