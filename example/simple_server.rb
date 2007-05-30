require 'nodewrap'

def foo
  puts 'foo!'
end

m = Object.method(:foo)
n = m.body
Marshal.dump(n, $stdout)
$stdout.flush

