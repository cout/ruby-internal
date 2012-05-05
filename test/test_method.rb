require 'test/unit'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/method'

require "node_samples"

$stdout.sync = true
$stderr.sync = true

class TC_Method < Test::Unit::TestCase
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

  class MarshalMethodHelper
    include Foo
  end

  include Foo

  def test_method_node
    m = method(:foo)
    n = m.body
    if defined?(RubyVM::InstructionSequence) then
      # YARV
      assert n.class == RubyVM::InstructionSequence || # 1.9.2 and later
             n.class == Node::METHOD                   # 1.9.1 and earlier
    else
      # pre-YARV
      assert_equal Node::SCOPE, n.class
    end
  end

  def test_dump_method
    o = MarshalMethodHelper.new
    m = o.method(:foo)
    d = Marshal.dump(m)
  end

  def test_load_method
    o = MarshalMethodHelper.new
    m = o.method(:foo)
    d = Marshal.dump(m)
    m2 = Marshal.load(d)
    assert_equal m.call, m2.call
  end

  def test_dump_unbound_method
    o = MarshalMethodHelper.new
    u = o.method(:foo).unbind
    d = Marshal.dump(u)
  end

  def test_load_unbound_method
    o = MarshalMethodHelper.new
    u = o.method(:foo).unbind
    d = Marshal.dump(u)
    u2 = Marshal.load(d)
    m = u.bind(o)
    m2 = u.bind(o)
    assert_equal m.call, m2.call
  end

  def test_method_oid
    m = method(:foo)
    oid = m.method_oid
  end
end

if __FILE__ == $0 then
  # exit Test::Unit::AutoRunner.run
end

