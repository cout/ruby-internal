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

RUBY_SOURCE_DIR = dir
$".push('ruby_source_dir.rb')

ruby_include_path = File.join(RUBY_SOURCE_DIR, 'include')
if not File.exist?(ruby_include_path) or \
   not File.directory?(ruby_include_path) then
  # pre-YARV
  ruby_include_path = RUBY_SOURCE_DIR
end
RUBY_INCLUDE_DIR = ruby_include_path

output_dir = "cached/ruby-#{RUBY_VERSION}"
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
code, version #{RUBY_VERSION}, which are licensed under the Ruby
license.  For more information, please see the file COPYING.
END
end

def copy_from_ruby_dir(src, dir, output_dir)
  File.copy(File.join(dir, src), output_dir) rescue Errno::ENOENT
end

copy_from_ruby_dir('COPYING', dir, output_dir)
copy_from_ruby_dir('GPL', dir, output_dir)
copy_from_ruby_dir('LEGAL', dir, output_dir)
copy_from_ruby_dir('LGPL', dir, output_dir)

