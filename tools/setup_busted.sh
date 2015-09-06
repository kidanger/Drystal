#!/usr/bin/env sh

if [ -z "$1" ]; then
    TYPE=release
else
    TYPE=$1
fi

rm -rf .luarocks
rm -rf .rocks

ROOT=`pwd`
BUILD=$ROOT/build-native-$TYPE
ROCKVER=2.2.2

wget https://github.com/keplerproject/luarocks/archive/v${ROCKVER}.tar.gz || exit 1
tar zxpf v${ROCKVER}.tar.gz
rm luarocks-${ROCKVER}.tar.gz
mv luarocks-$ROCKVER .luarocks

cd .luarocks
sh configure --lua-version=5.3 --with-lua-bin=$BUILD/external --with-lua-include=$ROOT/external/lua/src --prefix=`pwd` --sysconfdir=`pwd`/luarocks --force-config || exit 1
make build || exit 1
make install || exit 1
cd ..

./.luarocks/bin/luarocks --tree=.rocks install busted || exit 1
./.luarocks/bin/luarocks --tree=.rocks install moonscript || exit 1

