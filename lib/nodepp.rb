require 'nodewrap'

class Node
  ##
  # Return a string containing an ascii-art tree of the node's
  # structure.
  #
  def pretty_print(s = '', prefix = '')
    s << "NODE_#{self.nd_type.to_s} at #{self.nd_file}:#{self.nd_line}\n"
    self.members.each_with_index do |member, idx|
      last = (idx == self.members.size-1)
      s << "#{prefix}#{(last ? '+-' : '|-')}#{member} = "
      value = self[member]
      case value
      when Node
        value.pretty_print(s, prefix + (last ? '  ' : '| '))
      else
        s << value.inspect + "\n"
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
  n = m.body
  puts n.pretty_print
end

