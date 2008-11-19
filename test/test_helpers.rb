$mini_unit_exit_code = 0

def disable_mini_unit_auto_run
  MiniTest::Unit.class_eval do
    alias :run_ :run
    def run(*args)
      return $mini_unit_exit_code
    end
  end
end

def run_all_tests_with_mini_unit
  begin
    test = MiniTest::Unit.new
    args = ARGV.dup
    args << '-v'
    $mini_unit_exit_code = test.run(args)
    exit($mini_unit_exit_code)
  ensure
    disable_mini_unit_auto_run
  end
end

def run_all_tests_with_test_unit
  tests = []
  ObjectSpace.each_object(Class) do |o|
    if o < Test::Unit::TestCase then
      tests << o
    end
  end

  suite = Test::Unit::TestSuite.new("RubyInternal")
  tests.each do |test|
    test.suite.tests.each do |testcase|
      suite << testcase
    end
  end

  require 'test/unit/ui/console/testrunner'

  verbose = nil
  begin
    verbose = Test::Unit::UI.const_get(:VERBOSE)
  rescue NameError
    verbose = Test::Unit::UI::Console::TestRunner.const_get(:VERBOSE)
  end

  result = Test::Unit::UI::Console::TestRunner.run(
      suite,
      verbose)
  exit(result.error_count + result.failure_count)
end

def run_all_tests
  if defined?(MiniTest) then
    run_all_tests_with_mini_unit
  else
    run_all_tests_with_test_unit
  end
end

