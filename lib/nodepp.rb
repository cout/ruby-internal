require 'nodewrap'

class Node
  def pretty_print(indent = 0)
    s = "node type #{self.nd_type}\n"
    indent += 2
    self.members.each do |member|
      s += "#{' ' * indent}#{member} = "
      value = self[member]
      case value
      when Node
        s += value.pretty_print(indent + 2)
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

