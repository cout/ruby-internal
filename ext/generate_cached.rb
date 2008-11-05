require 'rubypp'
require 'fileutils'

dir = ARGV[0]
$:.unshift(dir)

# Unset RUBY_VERSION
class Object
  remove_const :RUBY_VERSION
end

# Figure out the Ruby version and set RUBY_VERSION to fool the scripts
# we'll be calling
File.open(File.join(dir, 'version.h')) do |version_h|
  version_h.each_line do |line|
    # #define RUBY_VERSION "1.6.4"
    if line =~ /#define RUBY_VERSION \"(.*?)\"/ then
      RUBY_VERSION = $1
    end
  end
end

# Set RUBY_SORUCE_DIR to the directory specified on the command line
RUBY_SOURCE_DIR = dir
$".push('ruby_source_dir.rb')

ruby_include_path = File.join(RUBY_SOURCE_DIR, 'include')
if not File.exist?(ruby_include_path) or \
   not File.directory?(ruby_include_path) then
  # pre-YARV
  ruby_include_path = RUBY_SOURCE_DIR
end
RUBY_INCLUDE_DIR = ruby_include_path

# Figure out where we're going to spit output
output_dir = File.expand_path("cached/ruby-#{RUBY_VERSION}")
basedir = File.expand_path(File.dirname(__FILE__))

def generate_cached_rpp(input_rpp, output_dir)
  FileUtils.mkdir_p(output_dir)
  input_dir = File.dirname(input_rpp)
  base = File.basename(input_rpp.gsub(/(.*)\..*/, "\\1"))
  out = File.join(output_dir, base)
  orig_dir = Dir.pwd
  begin
    Dir.chdir(input_dir)
    rubypp(input_rpp, out)
  ensure
    Dir.chdir(orig_dir)
  end
end

def recursively_generate_cached_rpps(input_dir, output_dir)
  Dir[File.join(input_dir, '*')].each do |file|
    next if file == '.' or file == '..'
    if File.directory?(file) then
      recursively_generate_cached_rpps(
          file,
          File.join(output_dir, File.basename(file)))
    end
  end

  Dir[File.join(input_dir, '*.rpp')].each do |rpp|
    generate_cached_rpp(rpp, output_dir)
  end
end

$stdout.puts "Generating cached files for ruby-#{RUBY_VERSION}"

recursively_generate_cached_rpps(
    File.join(basedir, 'internal'),
    File.join(output_dir, 'internal'))

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
if ruby_version_code >= 190 then
  yarv_header_output_dir =
    File.join(output_dir, 'internal', 'yarv-headers')
  FileUtils.mkdir_p(yarv_header_output_dir)
  FileUtils.cp(Dir["#{RUBY_SOURCE_DIR}/*.h"], yarv_header_output_dir)
end

