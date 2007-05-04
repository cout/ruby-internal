require 'rubypp'
require 'ftools'

dir = ARGV[0]
$:.unshift(dir)

class Object
  remove_const :RUBY_VERSION
end

File.open(File.join(dir, 'version.h')) do |version_h|
  version_h.each_line do |line|
    # #define RUBY_VERSION "1.6.4"
    if line =~ /#define RUBY_VERSION \"(.*?)\"/ then
      RUBY_VERSION = $1
    end
  end
end

require "#{dir}/rbconfig"
$".push('rbconfig.rb')
c = Config::CONFIG
ruby_version = [c['MAJOR'], c['MINOR'], c['TEENY']].join('.')

RUBY_SOURCE_DIR = dir
$".push('ruby_source_dir.rb')

output_dir = "cached/ruby-#{ruby_version}"
Dir.mkdir("cached") rescue Errno::EEXIST
Dir.mkdir(output_dir) rescue Errno::EEXIST

Dir['*.rpp'].each do |rpp|
  base = rpp.gsub(/(.*)\..*/, "\\1")
  out = File.join(output_dir, base)
  rubypp(rpp, out)
end

File.open(File.join(output_dir, 'README'), 'w') do |readme|
  readme.puts <<END
The files in this directory have been generated from the ruby source
code, version #{ruby_version}, which are licensed under the Ruby
license.  For more information, please see the file COPYING.
END
end

def copy_from_ruby_dir(src, output_dir)
  File.copy(File.join(dir, src), output_dir) rescue Errno::ENOENT
end

copy_from_ruby_dir('COPYING', output_dir)
copy_from_ruby_dir('GPL', output_dir)
copy_from_ruby_dir('LEGAL', output_dir)
copy_from_ruby_dir('LGPL', output_dir)

