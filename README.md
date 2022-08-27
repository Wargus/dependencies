This repository provides copies of all dependencies for Stratagus, so as to be
independent of other repositories vanishing.

There are two kinds, let's call them "frozen" and "updated". Frozen dependencies
are in a folder with a version number, they do not track a specific remote, and
would be created when we depend on some library that is no longer updated
upstream. This is the case currently for "lua-5.1.5".

For "updated" dependencies we instead track some upstream. See the update.sh
script for which these currently are. The update.sh script encodes the latest
revisions we build against and it makes it easy to update the inlined sources.
