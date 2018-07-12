iRODS-legacy
------------

This is the 3.3.1 version of iRODS, which is used in Lyon and is slightly
modified so that it builds with newer versions of GCC (5+).

Installation
~~~~~~~~~~~~

    cd iRODS
    ./scripts/configure
    make
    mkdir -p $HOME/opt/bin
    cp clients/icommands/bin/* $HOME/opt/bin

Questions and feedback to Tamas Gal <tgal@km3net.de>.
