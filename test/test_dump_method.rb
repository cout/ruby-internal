require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/method'
require 'internal/noex'
require 'internal/module'

require 'node_samples'

$stdout.sync = true
$stderr.sync = true

class TC_Dump_Module < Test::Unit::TestCase
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

  Tmp_Foo = nil

  class TestClassBase
    # for testing super()
    def foo
      return 42
    end
  end

  class TestClass < TestClassBase
    include Foo
    FOO = 1
    @foo = 2
    @@foo = 3

    # Make sure we have enough instance variables to exceed
    # ROBJECT_EMBED_LEN_MAX
    @bar1 = 1
    @bar2 = 2
    @bar3 = 3

    class << self
      FOO = 4
      @foo = 5
    end

    # for testing cvar
    # (can't be in base class due to change in 1.9 of class var lookup)
    @@a = 42
  end

  @@test_class = TestClass

  def initialize(test_method_name)
    # TODO: This seems to be the only way to get tests defined with #
    # define_method to run on 1.8.1 and earlier.
    @method_name = test_method_name
    @test_passed = true
  end

  extend Test::Unit::Assertions

  Method_Node_Samples.each do |node_name, sample_code|
    p = proc {
      c = TestClass.dup
      c.class_eval <<-END_DEF
        def foo
          #{sample_code}
        end
      END_DEF
      TC_Dump_Module.const_set("TestClass__#{node_name}", c)

      o = c.new
      m = o.method(:foo)
      n = m.body
      # puts "dumping first method"
      # p sample_code
      # pp o.method(:foo).body
      d = Marshal.dump(n)
      n2 = Marshal.load(d)

      klass2 = Class.new(TestClassBase)
      klass2.class_eval do
        # for testing cvar
        # (can't be in base class due to change in 1.9 of class var lookup)
        @@a = 42
        add_method(:foo, n2, Noex::PUBLIC)
      end
      obj1 = o
      obj2 = klass2.new

      orig_result = nil
      orig_exc = nil
      begin
        orig_result = o.foo() { 17 }
      rescue Exception => exc
        orig_exc = exc
      end

      # puts "getting obj2 method"
      # m = obj2.method(:foo)
      # puts "getting obj2 method body"
      # b = m.body
      # puts "getting type"
      # p b.class
      # p b.members
      # p b.rval
      # p b.next
      # p b.tbl
      # puts "dumping second method"
      # pp obj2.method(:foo).body
      dup_result = nil
      dup_exc = nil
      begin
        dup_result = obj2.foo() { 17 }
      rescue Exception => exc
        dup_exc = exc
      end

      assert_equal orig_exc, dup_exc
      assert_equal orig_result, dup_result
    }
    define_method "test_dump_method_#{node_name}", p
  end
end

if __FILE__ == $0 then
  exit Test::Unit::AutoRunner.run
end

