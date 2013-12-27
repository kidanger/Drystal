#!/bin/sh -x

PREFIX=$1
if [ -z "$1" ]; then
	PREFIX=""
fi
echo Installing drystal at $PREFIX

BIN=$PREFIX/usr/bin
LIB=$PREFIX/usr/lib64
SHARE=$PREFIX/usr/share/drystal

BUILD=build-native-release

test -d $BIN || mkdir -p $BIN
test -d $LIB || mkdir -p $LIB
test -d $SHARE || mkdir -p $SHARE

cp $BUILD/src/drystal $BIN/
cp $BUILD/external/liblua-drystal.so $LIB/

for ext in `ls $BUILD/extensions`
do
	test -d $BUILD/extensions/${ext} \
	-a -f $BUILD/extensions/${ext}/lib${ext}.so &&
		cp $BUILD/extensions/$ext/lib${ext}.so $SHARE/${ext}.so
done

for lua in `ls data/*.lua`
do
	cp $lua $SHARE
done

echo "#!/bin/sh -x" > remove.sh
echo "rm $BIN/drystal" >>remove.sh
echo "rm $LIB/liblua-drystal.so" >>remove.sh
for ext in `ls $BUILD/extensions`
do
	test -d $BUILD/extensions/${ext} \
	-a -f $BUILD/extensions/${ext}/lib${ext}.so &&
		echo "rm $SHARE/${ext}.so" >>remove.sh
done
for lua in `ls data/*.lua`
do
	echo "rm $SHARE/$(basename $lua)" >>remove.sh
done
echo "rmdir $SHARE" >>remove.sh
chmod +x remove.sh

