begin
  require 'test/unit'
  HAVE_TESTUNIT = true
rescue LoadError
  puts "WARNING: Test::Unit not installed; skipping tests"
  HAVE_TESTUNIT = false
end

if HAVE_TESTUNIT then
  $:.unshift('ext')
  load 'test/test.rb'
end

