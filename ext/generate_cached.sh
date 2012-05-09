#!/bin/sh

dir="$1"

if [ -z "$dir" ]; then
  dir=~/.rvm/src
fi

if [ ! -d "$dir" ]; then
  echo "$dir not found"
  echo "Usage: $0 <dir>"
  echo "where <dir> is the directory where the untarred ruby source archives can be found."
  exit 1
fi

dirs=`ls -1 -d "$dir"/ruby-?.?.? "$dir"/ruby-?.?.?-p*`
dirs=`echo $dirs | sort`

for dir in $dirs; do
  ruby generate_cached.rb "$dir"
done

