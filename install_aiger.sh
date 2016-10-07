#!/bin/bash

echo "Installing AIGER library ..."

AIGER="aiger-1.9.4"
AIGER_ARCHIVE="$AIGER.tar.gz"

if [ ! -e "$AIGER_ARCHIVE" ];
then
    echo " Downloading AIGER library ..."
    wget http://fmv.jku.at/aiger/$AIGER_ARCHIVE
fi

echo " Unpacking AIGER library ..."
rm -rf $AIGER
tar -xzf $AIGER_ARCHIVE

# echo " Compiling AIGER library ..."
# (cd $AIGER; ./configure.sh && make)

echo " Copying library files ..."
cp $AIGER/aiger.h .
cp $AIGER/aiger.c .
