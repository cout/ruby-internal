require 'internal/node'

class Node
  def obfusc(begin_nodes=[])
    n = self

    # The outputted code must do the following:
    #   1. Evaluate each of the begin nodes (BEGIN {...}).
    #   2. Evaluate the main node.
    return <<-END
  require 'internal/node'
  if RUBY_VERSION != "#{RUBY_VERSION}" then
    $stderr.puts "Wrong Ruby version; please use #{RUBY_VERSION}"
    exit 1
  end
  begin_nodes = Marshal.load(#{Marshal.dump(begin_nodes).inspect})
  n = Marshal.load(#{Marshal.dump(n).inspect})
  begin_nodes.each do |node|
    node.eval(self)
  end
  n.eval(self)
    END
  end
end

