#!/bin/bash

make clean
rm -rf debian
./scripts/debian/tar.pl
rm -rf debian
cp -r packaging/debian debian
debuild -us -uc
