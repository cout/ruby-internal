require 'rbconfig'

SRCDIR = Config::CONFIG['srcdir']
NODE_H_LOCATION = "#{SRCDIR}/node.h"

NODEINFO = Hash.new
File.open(NODE_H_LOCATION) do |node_h|
  loop do
    line = node_h.gets
    break if line.nil?
    case line
    when /^#define\s+nd_(\w+)\s+(.*)/
      NODEINFO[$1] = $2
    end
  end
end

