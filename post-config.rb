ruby_source_path = @config['ruby-source-path']
ruby_include_path = @config['ruby-include-path']

if ruby_source_path.nil? and ruby_include_path.nil? then

  cached_dir = "ext/cached/ruby-#{RUBY_VERSION}"
  if File.exist?(cached_dir) then
    puts "Ruby source path not specified; using generated files from #{cached_dir}"
  else
    $stderr.puts "ERROR: Source code for the ruby interpreter could not be found (perhaps you forgot --ruby-source-path?)"
    exit 1
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
  if not File.exist?(ruby_source_path) or
     not File.directory?(ruby_source_path) or
     not File.exist?(ruby_h) then
     $stderr.puts "ERROR: Could not find version.h at #{ruby_source_path}"
     exit 1
  end

  node_h = File.join(ruby_include_path, 'node.h')
  if not File.exist?(ruby_include_path) or
     not File.directory?(ruby_include_path) or
     not File.exist?(node_h) then
     $stderr.puts "ERROR: Could not find node.h at #{ruby_include_path}"
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

dive_into('ext') do
  command('make clean')
end

