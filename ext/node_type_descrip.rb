require 'ruby_version_code'
require 'ruby_source_dir'
require 'nodes'

NODE_TYPE_DESCRIPS = []

nodes = Nodes.new
nodes.each do |name, node|
  members = node['members'] || {}
  version_range = node['version_range']
  if version_range.includes?(RUBY_VERSION_CODE) then
    node_name = node['name'].upcase
    member_names = [
      (members.keys[0] || 'NONE').upcase,
      (members.keys[1] || 'NONE').upcase,
      (members.keys[2] || 'NONE').upcase,
    ]
    NODE_TYPE_DESCRIPS << [ node_name, *member_names ]
  end
end

NODE_TYPE_DESCRIPS.concat [
  [ 'LAST'        , 'NONE'  , 'NONE'  , 'NONE'  ], 
]

Node_Type_Descrip = Struct.new(:name, :node1, :node2, :node3)

NODE_TYPE_DESCRIPS.map! { |x| Node_Type_Descrip.new(*x) }

NEN_TO_NODE_TYPE = {}
NODE_TYPE_DESCRIPS.each do |descrip|
  [descrip.node1, descrip.node2, descrip.node3].each do |nen|
    NEN_TO_NODE_TYPE[nen] ||= []
    NEN_TO_NODE_TYPE[nen] << descrip.name
  end
end

