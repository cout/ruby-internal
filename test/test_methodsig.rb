require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'nodewrap'
require 'methodsig'

$stdout.sync = true
$stderr.sync = true

class TC_Methodsig < Test::Unit::TestCase
  def no_args_no_parens
  end

  def test_no_args_no_parens
    name = :no_args_no_parens
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [], sig.arg_names
    assert_equal Hash.new, sig.arg_info
    assert_equal "#{self.class.name}##{name}()", sig.to_s
  end

  def no_args_with_parens()
  end

  def test_no_args_with_parens
    name = :no_args_with_parens
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [], sig.arg_names
    assert_equal Hash.new, sig.arg_info
    assert_equal "#{self.class.name}##{name}()", sig.to_s
  end

  def one_arg(a)
  end

  def test_one_arg
    name = :one_arg
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a], sig.arg_names
    assert_equal Hash[:a=>"a"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a)", sig.to_s
  end

  def two_args(a, b)
  end

  def test_two_args
    name = :two_args
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a, :b], sig.arg_names
    assert_equal Hash[:a=>"a", :b=>"b"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a, b)", sig.to_s
  end
end
