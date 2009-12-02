require 'rake'

task :clean do
  ruby "setup.rb clean"
end

task :config do
  ruby "setup.rb config"
end

task :setup do
  ruby "setup.rb setup"
end

task :install do
  ruby "setup.rb install"
end

task :default => [ :config, :setup ]

