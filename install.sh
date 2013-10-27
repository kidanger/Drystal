#!/bin/sh -x

PREFIX=$1
if [ -z "$1" ]; then
	PREFIX=""
fi
echo Installing drystal at $PREFIX

BIN=$PREFIX/usr/bin
LIB=$PREFIX/usr/lib64
SHARE=$PREFIX/usr/share/drystal

test -d $BIN || mkdir -p $BIN
test -d $LIB || mkdir -p $LIB
test -d $SHARE || mkdir -p $SHARE

cp build-native/drystal $BIN/
cp build-native/external/liblua-drystal.so $LIB/

for ext in `ls build-native/extensions`
do
	cp build-native/extensions/$ext/main.so $SHARE/${ext}.so
done

for lua in `ls data/*.lua`
do
	cp $lua $SHARE
done

echo "#!/bin/sh -x" > remove.sh
echo "rm $BIN/drystal" >>remove.sh
echo "rm $LIB/liblua-drystal.so" >>remove.sh
for ext in `ls build-native/extensions`
do
	echo "rm $SHARE/${ext}.so" >>remove.sh
done
for lua in `ls data/*.lua`
do
	echo "rm $SHARE/$(basename $lua)" >>remove.sh
done
echo "rmdir $SHARE" >>remove.sh
chmod +x remove.sh

