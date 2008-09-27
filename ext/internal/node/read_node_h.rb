$: << '../..'
require 'ruby_source_dir'

NODE_H_LOCATION = "#{RUBY_INCLUDE_DIR}/node.h"
NODE_H_ALT_LOCATION = "#{RUBY_SOURCE_DIR}/node.h"

def open_alt(*files, &block)
  last_exc = nil
  files.each do |file|
    begin
      File.open(file, &block)
      return
    rescue Errno::ENOENT
      last_exc = $!
    end
  end
  raise last_exc
end

NODEINFO = Hash.new
open_alt(NODE_H_LOCATION, NODE_H_ALT_LOCATION) do |node_h|
  while line = node_h.gets do
    case line
    when /^#define\s+nd_(\w+)\s+(.*)/
      NODEINFO[$1] = $2
    end
  end
end

if __FILE__ == $0 then
  require 'pp'
  pp NODEINFO
end

