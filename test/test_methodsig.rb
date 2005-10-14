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

  def three_args(a, b, c)
  end

  def test_three_args
    name = :three_args
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a, :b, :c], sig.arg_names
    assert_equal Hash[:a=>"a", :b=>"b", :c=>"c"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a, b, c)", sig.to_s
  end

  def block_arg(&b)
  end

  def test_block_arg
    name = :block_arg
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:b], sig.arg_names
    assert_equal Hash[:b=>"&b"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(&b)", sig.to_s
  end

  def rest_arg(*r)
  end

  def test_rest_arg
    name = :rest_arg
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:r], sig.arg_names
    assert_equal Hash[:r=>"*r"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(*r)", sig.to_s
  end

  def block_and_rest_arg(*r, &b)
  end

  def test_block_and_rest_arg
    name = :block_and_rest_arg
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:r, :b], sig.arg_names
    assert_equal Hash[:r=>"*r", :b=>"&b"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(*r, &b)", sig.to_s
  end

  def default_arg(a = 1)
  end

  def test_default_arg
    name = :default_arg
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a], sig.arg_names
    assert_equal Hash[:a=>"a=1"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a=1)", sig.to_s
  end

  def default_and_nondefault_arg(a, b = 42)
  end

  def test_default_and_nondefault_arg
    name = :default_and_nondefault_arg
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a, :b], sig.arg_names
    assert_equal Hash[:a=>"a", :b=>"b=42"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a, b=42)", sig.to_s
  end

  def default_and_rest(a = 42, *r)
  end

  def test_default_and_rest
    name = :default_and_rest
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a, :r], sig.arg_names
    assert_equal Hash[:a=>"a=42", :r=>"*r"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a=42, *r)", sig.to_s
  end

  def default_and_rest_and_block(a = 42, *r, &b)
  end

  def test_default_and_rest_and_block
    name = :default_and_rest_and_block
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a, :r, :b], sig.arg_names
    assert_equal Hash[:a=>"a=42", :r=>"*r", :b=>"&b"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a=42, *r, &b)", sig.to_s
  end

  def nondefault_and_default_and_rest_and_block(a, b = 42, *r, &block)
  end

  def test_nondefault_and_default_and_rest_and_block
    name = :nondefault_and_default_and_rest_and_block
    sig = method(name).signature
    assert_equal self.class, sig.origin_class
    assert_equal name.to_s, sig.name
    assert_equal [:a, :b, :r, :block], sig.arg_names
    assert_equal Hash[:a=>"a", :b=>"b=42", :r=>"*r", :block=>"&block"], sig.arg_info
    assert_equal "#{self.class.name}##{name}(a, b=42, *r, &block)", sig.to_s
  end
end
