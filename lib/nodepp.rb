# Require this file to get a node pretty-printer.
#

require 'nodewrap'
require 'pp'

module Noex
  # Return a string representing the given integer as a noex value.
  #
  def stringify(noex)
    Noex.constants.each do |constant|
      return constant.to_s if const_get(constant) == noex
    end
    return noex.to_s
  end
  module_function :stringify
end

class Node
  # Return a string containing an ascii-art tree of the node's
  # structure.
  #
  def tree(s = '', prefix = '')
    s << "NODE_#{self.nd_type.to_s} at #{self.nd_file}:#{self.nd_line}\n"
    self.members.each_with_index do |member, idx|
      last = (idx == self.members.size-1)
      s << "#{prefix}#{(last ? '+-' : '|-')}#{member} = "
      value = self[member]
      if Node === value then
        value.tree(s, prefix + (last ? '  ' : '| '))
      elsif Object.const_defined?(:VM) and
            VM.const_defined?(:InstructionSequence) and
            VM::InstructionSequence === value then
        s << "<ISeq:#{value.self.name}@#{value.self.filename}>\n"
        d = value.disasm
        lines = d.split("\n")
        lines.each_with_index do |line, idx|
          if line =~ /^== disasm: (.*?)=/ then line = $1; end
          if line =~ /(.*)\s+\(\s*\d+\)/ then line = $1; end
          next if line =~ /^\|-----/
          last_line = (idx == lines.size-1)
          s << "#{prefix}#{last ? '  ' : '| '}#{(last_line ? '+-' : '|-')}#{line}\n"
        end
        # p value.local_table
      elsif member == 'noex' then
        s << Noex.stringify(value) + "\n"
      else
        s << value.inspect + "\n"
      end
    end
    return s
  end

  # Pretty-print node using Node#tree onto s, which can be a String or
  # IO.
  #
  def pretty_print(pp)
    pp.text(tree())
  end
end

if __FILE__ == $0 then
  def foo # :nodoc:
    1 + 1
  end

  m = method(:foo)
  n = m.body
  puts n.pretty_print
end

