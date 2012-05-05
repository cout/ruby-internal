require 'test/unit'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/node'
require 'internal/method'
require 'internal/module'
require 'internal/noex'

$stdout.sync = true
$stderr.sync = true

class TC_Node < Test::Unit::TestCase
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

  def test_node_eval
    node = Node.compile_string('1 + 1')
    assert_equal 2, node.eval(self)
  end

  def test_marshal_node
    m = method(:foo)
    n = m.body
    d = Marshal.dump(n)
    n2 = Marshal.load(d)
    case n2
    when Node
      # 1.9.1 and earlier
      assert_equal n.nd_file, n2.nd_file
      assert_equal n.nd_type, n2.nd_type
      assert_equal n.flags,   n2.flags
    when RubyVM::InstructionSequence
      # 1.9.2 and later
      assert_equal n.name, n2.name
      assert_equal n.filename, n2.filename
      assert_equal n.argc, n2.argc
      assert_equal n.arg_simple, n2.arg_simple
      assert_equal n.arg_rest, n2.arg_rest
      assert_equal n.arg_block, n2.arg_block
    else
      fail "Unexpeced type #{n2.class}"
    end
    klass = Class.new;
    klass.instance_eval do
      add_method(:foo, n2, Noex::PUBLIC)
    end
    obj = klass.new
    assert_equal 42, obj.foo
  end

  MAJOR = Config::CONFIG['MAJOR'].to_i
  MINOR = Config::CONFIG['MINOR'].to_i
  TEENY = Config::CONFIG['TEENY'].to_i
  RUBY_VERSION_CODE = MAJOR * 100 + MINOR * 10 + TEENY

  def test_ruby_eval_tree
    if RUBY_VERSION_CODE < 190 then
      assert Node === $ruby_eval_tree
    end
  end

  def test_ruby_eval_tree_begin
    if RUBY_VERSION_CODE < 190 then
      assert_equal nil, $ruby_eval_tree_begin
    end
  end
end

if __FILE__ == $0 then
  $: << '.' # TODO: hack
  require 'test_helpers'
  run_all_tests
end

