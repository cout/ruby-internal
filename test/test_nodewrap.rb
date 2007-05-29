require 'test/unit'
require 'test/unit/ui/console/testrunner'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'nodewrap'
require 'pp'
require 'nodepp'

require "node_samples"

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

  # For the cvar proc test
  @@a = 42

  def initialize(test_method_name)
    # TODO: This seems to be the only way to get tests defined with #
    # define_method to run on 1.8.1 and earlier.
    @method_name = test_method_name
    @test_passed = true
  end

  def test_node_eval
    node = Node.compile_string('1 + 1')
    assert_equal 2, node.eval(self)
  end

  if not Object.const_defined?(:VM) then
    define_method(:test_proc_body) do
      p = proc { 1 + 1 }
      assert_equal p.call, p.body.eval(self)
    end
  end

  def test_method_node
    m = method(:foo)
    n = m.body
    if Object.const_defined?(:VM) and
       VM.const_defined?(:InstructionSequence) then
      # YARV
      assert_equal Node::METHOD, n.class
    else
      # pre-YARV
      assert_equal Node::SCOPE, n.class
    end
  end

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

  def test_marshal_node
    m = method(:foo)
    n = m.body
    d = Marshal.dump(n)
    n2 = Marshal.load(d)
    assert_equal n.nd_file, n2.nd_file
    assert_equal n.nd_type, n2.nd_type
    assert_equal n.flags,   n2.flags
    klass = Class.new;
    klass.instance_eval do
      add_method(:foo, n2, Noex::PUBLIC)
    end
    obj = klass.new
    assert_equal 42, obj.foo
  end

  def test_marshal_proc
    p = proc { 1 + 1 }
    d = Marshal.dump(p)
    u = Marshal.load(d)
    p2 = u.bind(binding)
    assert_equal(p.call, p2.call)
  end

  class MarshalMethodHelper
    include Foo
  end

  def test_marshal_method
    o = MarshalMethodHelper.new
    m = o.method(:foo)
    d = Marshal.dump(m)
    m2 = Marshal.load(d)
    assert_equal m.call, m2.call
  end

  def test_marshal_unbound_method
    o = MarshalMethodHelper.new
    u = o.method(:foo).unbind
    d = Marshal.dump(u)
    u2 = Marshal.load(d)
    m = u.bind(o)
    m2 = u.bind(o)
    assert_equal m.call, m2.call
  end

  def test_ruby_eval_tree
    assert Node === $ruby_eval_tree
  end

  def test_ruby_eval_tree_begin
    assert_equal nil, $ruby_eval_tree_begin
  end

  def test_proc_unbind
    p = proc { 1 + 1 }
    u = p.unbind
    p2 = u.bind(binding)
    assert_equal(p.call, p2.call)
  end

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
    class << self
      FOO = 4
      @foo = 5
    end

    # for testing cvar
    # (can't be in base class due to change in 1.9 of class var lookup)
    @@a = 42
  end

  def test_marshal_class
    begin
      assert_equal 5, TestClass.singleton_class.instance_eval('@foo')
      assert_equal 2, TestClass.instance_eval('@foo')

      d = Marshal.dump(TestClass)

      # Thread critical should have been reset by the class restorer
      assert_equal false, Thread.critical

      # Make sure the class instance variables are still set
      assert_equal 5, TestClass.singleton_class.instance_eval('@foo')
      assert_equal 2, TestClass.instance_eval('@foo')

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

      # Thread critical should have been reset by the class restorer
      assert_equal false, Thread.critical

      # Also make sure the class instance variables are still set
      assert_equal 5, TestClass.singleton_class.instance_eval('@foo')
      assert_equal 2, TestClass.instance_eval('@foo')

    ensure
      Thread.critical = false
    end
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
      TC_Nodewrap.const_set("TestClass__#{node_name}", c)

      o = c.new
      m = o.method(:foo)
      n = m.body
      # puts "dumping first method"
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

  Proc_Node_Samples.each do |node_name, sample_code|
    p = proc {
      p_orig = eval <<-END_DEF
        proc {
          #{sample_code}
        }
      END_DEF

      #puts "dumping"
      d = Marshal.dump(p_orig)
      #puts "loading"
      u = Marshal.load(d)
      p_new = u.bind(binding)

      orig_result = nil
      orig_exc = nil
      begin
        orig_result = p_orig.call
      rescue Exception => exc
        orig_exc = exc
      end

      dup_result = nil
      dup_exc = nil
      begin
        dup_result = p_new.call
      rescue Exception => exc
        dup_exc = exc
      end

      assert_equal orig_exc, dup_exc
      assert_equal orig_result, dup_result
    }
    define_method "test_dump_proc_#{node_name}", p
  end
end

if __FILE__ == $0 then
  exit Test::Unit::AutoRunner.run
end

