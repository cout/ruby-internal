require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'nodewrap'
require 'as_expression'

dir = File.dirname(__FILE__)
require "#{dir}/expression_samples"

$stdout.sync = true
$stderr.sync = true

require 'pp'
require 'nodepp'

class TC_As_Expression < Test::Unit::TestCase
  extend Test::Unit::Assertions

  EXPRESSION_SAMPLES.each do |name, code|
    p = proc {
      p_orig = eval("proc { #{code} }")
      code_new = p_orig.body.as_expression
      p_new = eval("proc { #{code_new} }")
      result_orig = result_new = nil
      exc_orig = exc_new = nil
      begin; result_orig = p_orig.call; rescue; exc_orig = $!; end
      begin; result_new = p_new.call; rescue; exc_new = $!; end
      assert_equal(result_orig, result_new)
      assert_equal(exc_orig.class, exc_new.class)
      if exc_orig and exc_new then
        assert_equal(exc_orig.message, exc_new.message)
      end
    }
    define_method "test_#{name}", p
  end

  def initialize(test_method_name)
    # TODO: This seems to be the only way to get tests defined with #
    # define_method to run on 1.8.1 and earlier.
    @method_name = test_method_name
    @test_passed = true
  end

  def method_no_args
  end

  def test_method_no_args_as_expression
    m = method(:method_no_args)
    assert_equal 'def method_no_args(); nil; end', m.as_expression
  end

  def method_one_arg(a)
  end

  def test_method_one_arg_as_expression
    m = method(:method_one_arg)
    assert_equal 'def method_one_arg(a); nil; end', m.as_expression
  end

  def method_two_args(a, b)
  end

  def test_method_two_args_as_expression
    m = method(:method_two_args)
    assert_equal 'def method_two_args(a, b); nil; end', m.as_expression
  end

  def method_rest_arg(*rest)
  end

  def test_method_rest_arg_as_expression
    m = method(:method_rest_arg)
    assert_equal 'def method_rest_arg(*rest); nil; end', m.as_expression
  end

  def method_block_arg(&block)
  end

  def test_method_block_arg_as_expression
    m = method(:method_block_arg)
    assert_equal 'def method_block_arg(&block); nil; end', m.as_expression
  end

  def method_rest_and_block_arg(*rest, &block)
  end

  def test_method_rest_and_block_arg_as_expression
    m = method(:method_rest_and_block_arg)
    assert_equal 'def method_rest_and_block_arg(*rest, &block); nil; end', m.as_expression
  end

  def method_two_args_and_rest_and_block_arg(a, b, *rest, &block)
  end

  def test_method_two_args_and_rest_and_block_arg_as_expression
    m = method(:method_two_args_and_rest_and_block_arg)
    assert_equal 'def method_two_args_and_rest_and_block_arg(a, b, *rest, &block); nil; end', m.as_expression
  end

  def method_with_body(a, b)
    a + b
  end

  def test_method_with_body_as_expression
    m = method(:method_with_body)
    assert_equal 'def method_with_body(a, b); a + b; end', m.as_expression
  end

  def test_proc_no_args_as_expression
    p = proc { }
    assert_equal 'proc { }', p.as_expression
  end

  def test_proc_empty_args_as_expression
    p = proc { || }
    assert_equal 'proc { || }', p.as_expression
  end

  def test_proc_one_arg_as_expression
    p = proc { |a| }
    assert_equal 'proc { |a| }', p.as_expression
  end

  def test_proc_one_array_arg_as_expression
    p = proc { |a,| }
    assert_equal 'proc { |a,| }', p.as_expression
  end

  def test_proc_two_args_as_expression
    p = proc { |a, b| }
    assert_equal 'proc { |a, b| }', p.as_expression
  end

  def test_proc_rest_arg_as_expression
    p = proc { |*rest| }
    assert_equal 'proc { |*rest| }', p.as_expression
  end

  def test_proc_two_args_and_rest_arg_as_expression
    p = proc { |a, b, *rest| }
    assert_equal 'proc { |a, b, *rest| }', p.as_expression
  end

  def test_proc_with_body_as_expression
    p = proc { |a, b| a + b }
    assert_equal 'proc { |a, b| a + b }', p.as_expression
  end

  def setup
    @foo = 42
  end

  # 1.7 and later
  def foo(*args)
    return args
  end

  def remove_foo
    self.class.remove_foo
  end

  # 1.6
  def self.foo(*args)
    return args
  end

  def self.remove_foo
    self.class_eval { remove_const(:FOO) if const_defined?(:FOO) }
  end

  @@foo = 10

  FOO = 57
end

if __FILE__ == $0 then
  if Test::Unit.const_defined?(:AutoRunner) then
    exit Test::Unit::AutoRunner.run(__FILE__)
  else
    if ARGV.empty? then
      suite = TC_As_Expression.suite
    else
      suite = Test::Unit::TestSuite.new('TC_As_Expression')
      TC_As_Expression.suite.tests.each do |test|
        ARGV.each do |arg|
          suite << test if /#{arg}/ =~ test.name
        end
      end
    end
    result = Test::Unit::UI::Console::TestRunner.run(suite)
    exit(result.error_count + result.failure_count)
  end
end

