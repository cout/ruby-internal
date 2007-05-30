require 'nodewrap'

Marshal.load($stdin)

t = M::Triangle.new(3, 4, 5)
puts "The area of #{t} is: #{t.area}"

