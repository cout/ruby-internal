begin
  require 'fileutils'
rescue LoadError
  require 'ftools'
  FileUtils = File
end

# Make sure we look in `pwd`/ext before system directories (for
# mkmf-ruby-internal.rb)
ext_dir = File.join(Dir.pwd, 'ext')
$: << ext_dir
ENV['RUBYLIB'] ||= ''
ENV['RUBYLIB'] += ext_dir

FileUtils.rm_f 'ext/Makefile'

ruby_source_path = config('ruby-source-path')
ruby_include_path = config('ruby-include-path')

set_config('cached-files', 'no')

# If ruby source path was specified, set paths
if ruby_source_path != '' then

  if ruby_include_path == '' then
    ruby_include_path = File.join(ruby_source_path, 'include', 'ruby')
    if not File.exist?(ruby_include_path) or \
       not File.directory?(ruby_include_path) then
      # pre-YARV
      ruby_include_path = ruby_source_path
    end

    set_config('ruby-include-path', ruby_include_path)
  end

  @config.config_opt << "--ruby-source-path=#{@config['ruby-source-path']}"
  @config.config_opt << "--ruby-include-path=#{@config['ruby-include-path']}"
end

# If ruby source path is blank, use cached files
if ruby_source_path == '' then
  set_config('cached-files', 'yes')
  cached_dir = "ext/cached/ruby-#{RUBY_VERSION}"
  puts "Looking for cached files in #{Dir.pwd}/#{cached_dir}"
  if File.exist?(cached_dir) then
    puts "Ruby source path not specified; using generated files from #{cached_dir}"
  else
    $stderr.puts "ERROR: Source code for the ruby interpreter could not be found (perhaps you forgot --ruby-source-path?)"
    exit 1
  end

  @config.config_opt << "--enable-cached-files"

  # If running on YARV, copy the cached yarv-headers directory
  ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
  if ruby_version_code >= 190 then
    cached_dir = File.join('cached', "ruby-#{RUBY_VERSION}")
    dest_dir = "ext/internal/yarv-headers"
    FileUtils.mkdir_p(dest_dir)
    FileUtils.cp(Dir["ext/#{cached_dir}/internal/yarv-headers/*"], dest_dir)
  end

else

  ruby_h = File.join(ruby_source_path, 'eval.c')
  if not File.exist?(ruby_source_path) or
     not File.directory?(ruby_source_path) or
     not File.exist?(ruby_h) then
     $stderr.puts "ERROR: Could not find the ruby source code at #{ruby_source_path}"
     exit 1
  end

  version_h = File.join(ruby_source_path, 'version.h')
  if not File.exist?(ruby_h) then
     $stderr.puts "ERROR: Could not find version.h at #{version_h}"
     exit 1
  end

  node_h = File.join(ruby_include_path, 'node.h')
  node_h_alt = File.join(ruby_source_path, 'node.h')
  if not File.exist?(node_h) and
     not File.exist?(node_h_alt) then
     $stderr.puts "ERROR: Could not find node.h at #{node_h} or #{node_h_alt}"
     exit 1
  end

  File.open(version_h) do |vh|
    found_version = false
    vh.each_line do |line|
      if line =~ /#define\s+RUBY_VERSION\s+\"(.*?)\"/
        found_version = true
        if RUBY_VERSION != $1 then
          $stderr.puts "Wrong version for source; expected #{RUBY_VERSION} but got #{$1}"
          exit 1
        end
      end
    end
    if not found_version then
      $stderr.puts "ERROR: Could not determine ruby version from version.h"
      exit 1
    end
  end

end

