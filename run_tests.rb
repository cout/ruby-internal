def run_tests
  begin
    require 'test/unit'
  rescue LoadError
    puts "WARNING: Test::Unit not installed; skipping tests"
    return
  end

  $:.unshift('ext')
  load 'test/test.rb'
end

if __FILE__ == $0 then
  run_tests()
end

