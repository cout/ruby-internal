require 'yaml'

class NodeVersionRange
  def initialize(str)
    if not str or str =~ /^\s*$/ then
      @inclusive_start = true
      @range_start = 0
      @range_end = 1.0 / 0
      @inclusive_end = true
    elsif str =~ / ([\[(]) (.*?), \s* (.*?) ([\])]) /x then
      @inclusive_start = $1 == '['
      @range_start = $2.to_i
      @range_end = $3 == 'oo' ? (1.0 / 0) : $3.to_i
      @inclusive_end = $4 == ']'
    else
      raise "Invalid version range: #{str}"
    end
  end

  def includes?(version_code)
    if @inclusive_start then
      return false if version_code < @range_start
    else
      return false if version_code <= @range_start
    end

    if @inclusive_end then
      return false if version_code > @range_end
    else
      return false if version_code >= @range_end
    end

    return true
  end
end

class Nodes
  NodeInfo = Struct.new(:name, :doc, :members, :version_range)
  MemberInfo = Struct.new(:doc, :type)

  def initialize(filename = nil)
    if not filename then
      dir = File.dirname(__FILE__)
      filename = File.join(dir, 'nodes.yaml')
    end
    nodes = File.open(filename) { |io| YAML.load(io) }

    @nodes = {}
    nodes.each do |name, node|
      members = {}
      # TODO: is there any way to enforce order here?
      (node['members'] || {}).each do |member_name, member|
        member ||= {}
        members[member_name] = MemberInfo.new(
            member['doc'],
            member['type'])
      end
      version_range = NodeVersionRange.new(node['version'])
      @nodes[name] = NodeInfo.new(
          name.gsub(/\(.*\)/, ''),
          node['doc'],
          members,
          version_range)
    end
  end

  def each(&block)
    @nodes.sort.each(&block)
  end

  def [](name)
    @nodes[name]
  end

  def include?(name)
    @nodes.include?(name)
  end
end

