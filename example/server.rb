require 'nodewrap'

def foo
  puts 'foo!'
end

m = Object.method(:foo)
n = Node.method_node(m)
Marshal.dump(n, $stdout)
$stdout.flush

