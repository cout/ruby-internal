def run_tests
  begin
    require 'test/unit'
  rescue LoadError
    puts "WARNING: Test::Unit not installed; skipping tests"
    return
  end

  $:.unshift('ext')
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

  # TODO: this doesn't look quite right to me...
  exitval = 0
  tests.each do |test|
    result = Test::Unit::UI::Console::TestRunner.run(
        test.suite,
        verbose)
    exitval += result.error_count + result.failure_count
  end
end

p __FILE__
p $0
if __FILE__ == $0 then
  run_tests()
end

