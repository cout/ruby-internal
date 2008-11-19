require 'test/unit'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/module'
require 'internal/object'

$stdout.sync = true
$stderr.sync = true

# Instance methods defined in Class get returned by
# rb_class_instance_methods, though they are not in the class's method
# table.  Having this here ensures that we don't accidentally try to
# dump methods that aren't in the class we are trying to dump.
class Class
  def xyzzy
  end
end

class TC_Dump_Class < Test::Unit::TestCase
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

  def setup
    self.class.instance_eval do
      remove_const :TestClass if defined?(TestClass)
      const_set :TestClass, @@test_class
    end
  end

  def test_marshal_class
    begin
      assert_equal 5, TestClass.singleton_class.instance_eval('@foo')
      assert_equal 2, TestClass.instance_eval('@foo')

      d = Marshal.dump(TestClass)

      # Thread critical should have been reset by the class restorer
      if not defined?(RubyVM) then
        assert_equal false, Thread.critical
      end

      # Make sure the class instance variables are still set
      assert_equal 5, TestClass.singleton_class.instance_eval('@foo')
      assert_equal 2, TestClass.instance_eval('@foo')

      orig_test_class = TestClass
      orig_test_class_iv = TestClass.instance_variables
      self.class.instance_eval { remove_const :TestClass }

      c = Marshal.load(d)
      assert_equal(TestClass, c)
      assert_not_equal(orig_test_class, c)
      assert_equal Class, c.class

      a = c.ancestors
      assert a.include?(Object)
      assert a.include?(Kernel)
      assert a.include?(Foo)
      assert a.include?(c)

      # Class instance variables
      assert_equal orig_test_class_iv.sort, c.instance_variables.sort
      result = c.instance_eval do
        @foo
      end
      assert_equal 2, result

      # Constant lookup (constants are really just special instance
      # variables)
      assert_equal 1, c::FOO

      # Constant lookup in singleton class
      result = class << c
        FOO
      end
      assert_equal 4, result

      # Singleton class instance variable lookup
      result = class << c
        @foo
      end
      assert_equal 5, result

      # Class variable lookup
      # This is a little bit messy, but it was the only way I could figure
      # to get at @@foo
      self.class.instance_eval do
        remove_const :Tmp_Foo
        const_set :Tmp_Foo, c
      end
      eval "class Tmp_Foo; $at_at_foo = @@foo; end"
      result = $at_at_foo
      assert_equal 3, result

      # Test method call
      f = c.new
      assert_equal 42, f.foo()
      assert_equal 42*42, f.foo(42)

      # Thread critical should have been reset by the class restorer
      if not defined?(RubyVM) then
        assert_equal false, Thread.critical
      end

      # Also make sure the class instance variables are still set
      assert_equal 5, TestClass.singleton_class.instance_eval('@foo')
      assert_equal 2, TestClass.instance_eval('@foo')

    ensure
      if not defined?(RubyVM) then
        Thread.critical = false
      end
    end
  end
end

if __FILE__ == $0 then
  require 'test/unit/ui/console/testrunner'

  if Test::Unit.const_defined?(:AutoRunner) then
    exit Test::Unit::AutoRunner.run
  else
    if ARGV.empty? then
      suite = TC_Dump_Class.suite
    else
      suite = Test::Unit::TestSuite.new('TC_Dump_Class')
      TC_Dump_Class.suite.tests.each do |test|
        ARGV.each do |arg|
          suite << test if /#{arg}/ =~ test.name
        end
      end
    end
    result = Test::Unit::UI::Console::TestRunner.run(suite)
    exit(result.error_count + result.failure_count)
  end
end

