require 'find'
require 'ftools'

project = File.basename(File.expand_path('.'))
version = `cat VERSION`.chomp
package = "#{project}-#{version}"

files = []
Find.find('.') do |filename|
  if filename =~ /\.\/(.*)/ then
    filename = $1
  end
  if filename =~ /\/CVS$/ then
    dir = File.dirname(filename)
    File.open(File.join(filename, 'Entries')) do |entries|
      entries.each_line do |entry|
        info = entry.split('/')
        filename = info[1]
        next if filename.nil?
        files.push(File.join(package, File.join(dir, filename)))
      end
    end
  end
end

File.mkpath('packages')
Dir.chdir('packages')
File.open("#{package}.list", "w") do |out|
  out << files.join("\n")
end
if File.exists?(package) then
  File.unlink(package)
end
File.symlink('..', "#{package}")
system("tar cv -T #{package}.list -f #{package}-#{version}.tar.gz")

