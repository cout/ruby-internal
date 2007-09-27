require 'ruby_source_dir'

NODE_H_LOCATION = "#{RUBY_INCLUDE_DIR}/node.h"

NODEINFO = Hash.new
File.open(NODE_H_LOCATION) do |node_h|
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

