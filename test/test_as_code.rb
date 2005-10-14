require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'nodewrap'
require 'as_code'

dir = File.dirname(__FILE__)
require "#{dir}/expression_samples"

$stdout.sync = true
$stderr.sync = true

class TC_As_Code < Test::Unit::TestCase
  extend Test::Unit::Assertions

  EXPRESSION_SAMPLES.each do |name, code|
    p = proc {
      p_orig = eval("proc { #{code} }")
      code_new = p_orig.body.as_code
      p_new = eval("proc { #{code_new} }")
      result_orig = result_new = nil
      exc_orig = exc_new = nil
      begin; result_orig = p_orig.call; rescue; exc_orig = $!; end
      begin; result_new = p_new.call; rescue; exc_new = $!; end
      assert_equal(exc_orig.class, exc_new.class)
      if exc_orig and exc_new then
        assert_equal(exc_orig.message, exc_new.message)
      end
      assert_equal(result_orig, result_new)
    }
    define_method "test_#{name}", p
  end

  def initialize(test_method_name)
    # TODO: This seems to be the only way to get tests defined with #
    # define_method to run on 1.8.1 and earlier.
    @method_name = test_method_name
    @test_passed = true
  end

  def setup
    @foo = 42
  end

  def foo(*args)
    return args
  end

  @@foo = 10

  FOO = 57
end

