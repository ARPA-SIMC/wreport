#!/bin/bash
set -eu

# Creating documentation in master
git checkout master
autoreconf -ifv
./configure
make
# Remove everything but .git and sphinx doc folders
find . \( -path ./.git -o -path ./python/apidocs \) -prune -o -type f -print0 | xargs -0 rm
# Remove local gh-pages branch (if exists) and re-create it
git show-ref --quiet --verify -- refs/heads/gh-pages && git branch -D gh-pages
git checkout --orphan gh-pages
# Moving docs in the root directory
mv python/apidocs/* .
# Add special file for Github pages
touch .nojekyll
# Add files and force push
git add .
git commit -m "Update gh-pages"
git push -f origin gh-pages
