begin
  require 'fileutils'
rescue LoadError
  require 'ftools'
  FileUtils = File
end

FileUtils.rm 'ext/Makefile'

ruby_source_path = config('ruby-source-path')
ruby_include_path = config('ruby-include-path')

if ruby_source_path then
  if not ruby_include_path then
    ruby_include_path = File.join(ruby_source_path, 'include', 'ruby')
    if not File.exist?(ruby_include_path) or \
       not File.directory?(ruby_include_path) then
      # pre-YARV
      puts "Could not find #{ruby_include_path}"
      ruby_include_path = ruby_source_path
    end

    set_config('ruby-include-path', ruby_include_path)
  end
end

@options['config-opt'] << "--ruby-source-path=#{@config['ruby-source-path']}"
@options['config-opt'] << "--ruby-include-path=#{@config['ruby-include-path']}"
