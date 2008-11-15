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

RUBY_SOURCE_DIR = config['ruby-source-path']
RUBY_INCLUDE_DIR = config['ruby-include-path']
USING_CACHED_FILES = config['cached-files'] == 'yes'

if __FILE__ == $0 then
  puts "Using cached files: #{USING_CACHED_FILES}"
  puts "Ruby source dir: #{RUBY_SOURCE_DIR}"
  puts "Ruby include dir: #{RUBY_INCLUDE_DIR}"
end

