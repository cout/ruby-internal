require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'nodewrap'

dir = File.dirname(__FILE__)
require "#{dir}/node_samples"

$stdout.sync = true
$stderr.sync = true

class TC_As_Expression < Test::Unit::TestCase
  def test_dummy
  end
end

