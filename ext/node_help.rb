require 'node_type_descrip'

NODE_HELP = { }
NODE_MEMBERS = { }

node_name = nil
help_lines = []
found_node_members = false

File.open('node_help.txt') do |node_help|
  node_help.each_line do |line|
    line.chomp!
    case line
    when /^=\s+(.*)\s+=$/
      if help_lines.size > 0 then
        while help_lines[-1] == '' do
          help_lines.pop
        end
        NODE_HELP[node_name] = help_lines
        help_lines = []
        found_node_members = false
      end

      node_name = $1
      NODE_MEMBERS[node_name] = { }

    when /^Members:$/
      help_lines << line
      found_node_members = true

    when /^\*/
      help_lines << line
      if found_node_members then
        if line =~ /^\* (.*?) \((.*?)\) - / then
          member_name = $1
          member_type = $2
          NODE_MEMBERS[node_name][member_name] = member_type
        else
          $stderr.puts "Warning: wrong format for node member: #{line.inspect}"
        end
      end

    else
      help_lines << line
    end
  end
end

extra_help_nodes = NODE_HELP.keys
NODE_TYPE_DESCRIPS.each do |descrip|
  node_name = descrip.name
  m1 = descrip.node1
  m2 = descrip.node2
  m3 = descrip.node3

  if not NODE_HELP[node_name] then
    $stderr.puts "Warning: no documentation for #{node_name}"
  else
    extra_help_nodes.delete(node_name)

    expected_members = [m1, m2, m3].delete_if { |m| m == 'NONE' }.sort.map { |m| m.downcase }
    members = NODE_MEMBERS[node_name].keys

    if expected_members != members then
      $stderr.puts "Warning: members don't match for #{node_name}: #{expected_members.inspect} != #{members.inspect}"
    end
  end
end

extra_help_nodes.each do |node_name|
  $stderr.puts "Warning: found documentation for unknown node #{node_name}"
end

if __FILE__ == $0 then
  require 'pp'
  pp NODE_HELP
end

