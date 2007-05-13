def run_tests
  begin
    require 'test/unit'
  rescue LoadError
    puts "WARNING: Test::Unit not installed; skipping tests"
    return
  end

  $:.unshift('ext')
  $:.unshift('lib')
  tests = Dir['test/test_*.rb']
  tests.each do |test|
    load test
  end

  verbose = nil
  begin
    verbose = Test::Unit::UI.const_get(:VERBOSE)
  rescue NameError
    verbose = Test::Unit::UI::Console::TestRunner.const_get(:VERBOSE)
  end
  
  tests = []
  ObjectSpace.each_object do |o|
    if Class === o and o < Test::Unit::TestCase then
      tests << o
    end
  end

  suite = Test::Unit::TestSuite.new("Nodewrap")
  tests.each do |test|
    test.suite.tests.each do |testcase|
      suite << testcase
    end
  end

  return Test::Unit::UI::Console::TestRunner.run(
      suite,
      verbose)
end

if __FILE__ == $0 then
  require 'timeout'
  result = nil
  timeout(600) { result = run_tests() }
  exit(result.error_count + result.failure_count)
end

