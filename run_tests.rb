require 'test/test_helpers'

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

  run_all_tests()
end

if __FILE__ == $0 then
  require 'timeout'
  result = nil
  timeout(600) { run_tests() }
end

