TEMPLATE         = subdirs
CONFIG          += ordered
SUBDIRS          = bncsutil libghost cli plugins

# set up correct paths
libghost.subdir  = src/libghost
plugins.subdir   = src/plugins
cli.subdir       = src/cli
bncsutil.subdir  = src/bncsutil

# dependencies
libghost.depends = bncsutil
cli.depends      = libghost
plugins.depends  = libghost
