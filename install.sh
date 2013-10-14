#!/bin/sh -x

LUA=/usr/local/lib/lua/5.2

cp build-native/drystal /usr/bin/
cp build-native/external/liblua-drystal.so /usr/lib64/

for ext in `ls build-native/extensions`
do
	cp build-native/extensions/$ext/main.so $LUA/${ext}.so
done

for lua in `ls data/*.lua`
do
	cp $lua $LUA
done

echo "#!/bin/sh -x" > remove.sh
echo "rm /usr/bin/drystal" >>remove.sh
echo "rm /usr/lib64/liblua-drystal.so" >>remove.sh
for ext in `ls build-native/extensions`
do
	echo "rm $LUA/${ext}.so" >>remove.sh
done
for lua in `ls data/*.lua`
do
	echo "rm $LUA/$(basename $lua)" >>remove.sh
done
chmod +x remove.sh

