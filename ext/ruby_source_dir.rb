wd = Dir.pwd()
Dir.chdir('..')
require 'install.rb'
config = ConfigTable.load
Dir.chdir(wd)

RUBY_SOURCE_DIR = config['ruby-source-path']

if __FILE__ == $0 then
  puts "Ruby source dir: #{RUBY_SOURCE_DIR}"
end

