require 'test/unit'
require 'rbconfig'

dir = File.dirname(__FILE__)
$:.unshift(dir) if not $:.include?(dir)
$:.unshift("#{dir}/../lib") if not $:.include?("#{dir}/../lib")
$:.unshift("#{dir}/../ext") if not $:.include?("#{dir}/../ext")

require 'internal/thread'

$stdout.sync = true
$stderr.sync = true

class TC_Thread < Test::Unit::TestCase
  def test_thread_cfp
    return if not defined?(RubyVM)

    cfp = Thread.current.cfp
  end
end

if __FILE__ == $0 then
  exit Test::Unit::AutoRunner.run
end

