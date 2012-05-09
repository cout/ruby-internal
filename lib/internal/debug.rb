require 'internal/node'
require 'internal/node/as_expression'
require 'internal/node/pp'

module Internal
  def printdebug(&block)
    puts "#{caller[0]}: #{block.body.as_expression} => #{block.call}"
  end
end

if __FILE__ == $0 then
  include Internal
  a = 41
  printdebug { a + 1 }
end

