require 'generate_rdoc'

generate_rdoc
system("rsync -r -P doc/* cout@rubyforge.org:/var/www/gforge-projects/ruby-internal")
