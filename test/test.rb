require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'nodewrap'

class TC_Nodewrap < Test::Unit::TestCase
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

  def test_method_node
    m = method(:foo)
    n = Node.method_node(m)
    assert_equal Node, n.class
  end

  def test_add_method
    m = method(:foo)
    n = Node.method_node(m)
    klass = Class.new;
    klass.instance_eval do
      add_method(:foo, n, 0)
    end
    obj = klass.new
    assert_equal 42, obj.foo
  end

  def test_marshal_node
    m = method(:foo)
    n = Node.method_node(m)
    d = Marshal.dump(n)
    n2 = Marshal.load(d)
    assert_equal n.nd_file, n2.nd_file
    assert_equal n.nd_type, n2.nd_type
    assert_equal n.flags,   n2.flags
    klass = Class.new;
    klass.instance_eval do
      add_method(:foo, n2, 0)
    end
    obj = klass.new
    assert_equal 42, obj.foo
  end

  class TestClass
    include Foo
    FOO = 1
    @foo = 2
    @@foo = 3
    class << self
      FOO = 4
      @foo = 5
    end
  end

  def test_marshal_class
    d = Marshal.dump(TestClass)
    c = Marshal.load(d)
    assert_equal Class, c.class
    a = c.ancestors
    assert a.include?(Object)
    assert a.include?(Kernel)
    assert a.include?(Foo)
    assert a.include?(c)

    # Constant lookup
    assert_equal 1, c::FOO

    # Constant lookup in singleton class
    result = class << c
      FOO
    end
    assert_equal 4, result

    # Class instance variable lookup
    result = c.instance_eval do
      @foo
    end
    assert_equal 2, result

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
  end
end

Test::Unit::UI::Console::TestRunner.run(TC_Nodewrap)

