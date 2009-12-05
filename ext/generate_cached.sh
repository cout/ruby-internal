#!/bin/sh

dir="$1"

if [ -z "$dir" ]; then
  echo "Usage: $0 <dir>"
  echo "where <dir> is the directory where the untarred ruby source archives can be found."
  exit 1
fi

for f in "$dir"/ruby-?.?.?; do
  ruby generate_cached.rb "$f"
done

