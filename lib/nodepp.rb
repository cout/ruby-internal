require 'nodewrap'

class Node
  def pretty_print(indent = 0)
    s = "#{self.nd_type}:\n"
    indent += 2
    self.members.each do |member|
      s += ' ' * indent + "#{member}: "
      value = self.send(member) # TODO
      case value
      when Node
        s += "\n" + value.pretty_print(indent + 2)
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
  n = Node.method_node(m)
  puts n.pretty_print
end

