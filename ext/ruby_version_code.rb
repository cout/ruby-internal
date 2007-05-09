require 'ruby_source_dir'

# We could use rbconfig, but it might not be generated yet (or
# correctly)

File.open(File.join(RUBY_SOURCE_DIR, 'version.h')) do |version_h|
  version_h.each_line do |line|
    # #define RUBY_VERSION "1.6.4"
    if line =~ /#define RUBY_VERSION \"(.*?)\.(.*?)\.(.*?)\"/ then
      major = $1.to_i
      minor = $2.to_i
      teeny = $3.to_i
      RUBY_VERSION_CODE = major * 100 + minor * 10 + teeny
    end
  end
end
