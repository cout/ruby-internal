require 'test/unit'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/module'
require 'internal/method'
require 'internal/noex'

$stdout.sync = true
$stderr.sync = true

class TC_Module < Test::Unit::TestCase
  module Foo
    def foo(n=1)
      # A fancy way to return 42
      if false then
        answer = 0
      else
        answer = 0
        (1..42).each do |i|
          answer += (i / i)
        end
      end
      return n * answer
    end
  end

  include Foo

  def test_add_method
    m = method(:foo)
    n = m.body
    klass = Class.new
    klass.instance_eval do
      add_method(:foo, n, Noex::PUBLIC)
    end
    obj = klass.new
    assert_equal 42, obj.foo
  end
end

if __FILE__ == $0 then
  exit Test::Unit::AutoRunner.run
end

