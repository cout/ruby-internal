require 'find'

begin
  require 'test/unit'
  HAVE_TESTUNIT = true
rescue LoadError
  puts "WARNING: Test::Unit not installed; skipping tests"
  HAVE_TESTUNIT = false
end

begin
  require 'rdoc/rdoc'
  HAVE_RDOC = true
rescue LoadError
  puts "WARNING: RDoc not installed; skipping generation of docs"
  HAVE_RDOC = false
end

if HAVE_TESTUNIT then
  $:.unshift('ext')
  load 'test/test.rb'
end

def list_files(dir, pattern)
  arr = []
  Find.find(dir) do |filename|
    if filename =~ pattern then
      arr.push(filename)
    end
  end
  return arr
end

if HAVE_RDOC then
  r = RDoc::RDoc.new
  rdoc_files = []
  rdoc_files.concat list_files('lib', /\.rb$/) if File.exist?('lib')
  rdoc_files.concat list_files('ext', /\.c$/)  if File.exist?('ext')
  r.document(rdoc_files)
end

