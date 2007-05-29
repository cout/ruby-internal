#!/bin/sh
RUBYLIB=$RUBYLIB:../lib:../ext
ruby server.rb | ruby client.rb

