require 'internal/node'
require 'internal/node/as_expression'
require 'internal/node/pp'

module Internal
  # Print an expression and its result, e.g.:
  #
  #   printdebug { 1 + 1 }
  #
  # will print:
  #
  #   test.rb:42: 1 + 1 => 2
  #
  def printdebug(&block)
    puts "#{caller[0]}: #{block.body.as_expression} => #{block.call}"
  end
end

if __FILE__ == $0 then
  include Internal
  a = 41
  printdebug { a + 1 }
end

