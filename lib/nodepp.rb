require 'nodewrap'

class Node
  def pretty_print(prefix = '')
    nd_type = self.nd_type
    nd_file = self.nd_file
    nd_line = self.nd_line
    s = "NODE_#{nd_type.to_s} (#{nd_type.to_i}) at #{nd_file}:#{nd_line}\n"
    self.members.each_with_index do |member, idx|
      last = (idx == self.members.size-1)
      s += "#{prefix}#{(last ? '+-' : '|-')}#{member} = "
      value = self[member]
      case value
      when Node
        s += value.pretty_print(prefix + (last ? '  ' : '| '))
      else
        s += value.inspect + "\n"
      end
    end
    return s
  end
end

if __FILE__ == $0 then
  def foo
    1 + 1
  end

  m = method(:foo)
  n = m.node
  puts n.pretty_print
end

