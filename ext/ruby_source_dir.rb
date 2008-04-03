wd = Dir.pwd()
metaconfig_dir = File.join(File.dirname(__FILE__), '..')
Dir.chdir metaconfig_dir
begin
  require 'setup.rb'
  require 'rbconfig'
  config = ConfigTable.new(Config::CONFIG)
  config.load_standard_entries
  config.load_script 'metaconfig'
  config.load_savefile
ensure
  Dir.chdir(wd)
end

RUBY_SOURCE_DIR = config['ruby-source-path'] || "cached/ruby-#{RUBY_VERSION}"
RUBY_INCLUDE_DIR = config['ruby-include-path'] || RUBY_SOURCE_DIR
USING_CACHED_FILES = (RUBY_SOURCE_DIR =~ /^cached/) ? true : false

if __FILE__ == $0 then
  puts "Using cached files: #{USING_CACHED_FILES}"
  puts "Ruby source dir: #{RUBY_SOURCE_DIR}"
  puts "Ruby include dir: #{RUBY_INCLUDE_DIR}"
end

