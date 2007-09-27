wd = Dir.pwd()
Dir.chdir('..')
begin
  require 'install.rb'
  config = ConfigTable.load
ensure
  Dir.chdir(wd)
end

RUBY_SOURCE_DIR = config['ruby-source-path']
RUBY_INCLUDE_DIR = config['ruby-include-path']

if __FILE__ == $0 then
  puts "Ruby source dir: #{RUBY_SOURCE_DIR}"
  puts "Ruby source dir: #{RUBY_INCLUDE_DIR}"
end

