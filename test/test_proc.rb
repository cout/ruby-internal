require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/proc'

$stdout.sync = true
$stderr.sync = true

class TC_Node < Test::Unit::TestCase
  def initialize(test_method_name)
    # TODO: This seems to be the only way to get tests defined with #
    # define_method to run on 1.8.1 and earlier.
    catch(:invalid_test) { super(test_method_name) }
  end

  def test_marshal_proc
    p = proc { 1 + 1 }
    d = Marshal.dump(p)
    u = Marshal.load(d)
    p2 = u.bind(binding)
    assert_equal(p.call, p2.call)
  end

  def test_proc_unbind
    p = proc { 1 + 1 }
    u = p.unbind
    p2 = u.bind(binding)
    assert_equal(p.call, p2.call)
  end

  if not defined?(RubyVM) then
    define_method(:test_proc_body) do
      p = proc { 1 + 1 }
      assert_equal p.call, p.body.eval(self)
    end
  end
end

if __FILE__ == $0 then
  exit Test::Unit::AutoRunner.run
end

