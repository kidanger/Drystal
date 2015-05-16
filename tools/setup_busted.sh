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

wget http://luarocks.org/releases/luarocks-2.2.1.tar.gz
tar zxpf luarocks-2.2.1.tar.gz
rm luarocks-2.2.1.tar.gz
mv luarocks-2.2.1 .luarocks

cd .luarocks
sh configure --lua-version=5.3 --with-lua-bin=$BUILD/external --with-lua-include=$ROOT/external/lua/src --prefix=`pwd`
make bootstrap
cd ..

./.luarocks/bin/luarocks --tree=.rocks install busted
./.luarocks/bin/luarocks --tree=.rocks install moonscript

