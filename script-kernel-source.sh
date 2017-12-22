#!/bin/sh
set -e
rm -rf linux-3.10.12
echo "Preparing kernel source tree under ./linux-3.10.12..."
echo "Extracting dl/linux-3.10.12 tar ball..."
tar -xjf dl/linux-3.10.12.tar.bz2
cd linux-3.10.12
echo "Copying driver files..."
cp -aL ../target/linux/generic/files/* .
cp -aL ../target/linux/lantiq/files/* .
echo "Applying generic patches.."
for patch in `ls ../target/linux/generic/patches-3.10/*.patch`
do
	patch -p1  < $patch
done
echo "Applying lantiq target patches.."
for patch in `ls ../target/linux/lantiq/patches-3.10/*.patch`
do
	patch -p1  < $patch
done
find .  -name \*.rej -or -name \*.orig  |xargs rm -rf
cd ..
