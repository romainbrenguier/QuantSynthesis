#!/bin/bash

echo "Installing CUDD utilities ..."

CUDD="cudd-2.5.0"
CUDD_ARCHIVE="$CUDD.tar.gz"

if [ ! -e "$CUDD_ARCHIVE" ];
then
  echo " Downloading CUDD utilities ..."
  wget ftp://vlsi.colorado.edu/pub/$CUDD_ARCHIVE
fi

echo " Unpacking CUDD utilities ..."
rm -rf $CUDD
tar -xzf $CUDD_ARCHIVE

echo " Patching CUDD..."
(cd $CUDD; patch -p0 -i ../cudd.patch)

echo " Compiling CUDD utilities ..."
(cd $CUDD; make; make objlib)
