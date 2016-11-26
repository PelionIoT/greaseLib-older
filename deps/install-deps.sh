#!/bin/bash

# http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  SELF="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done

DEPS_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
LOG=${DEPS_DIR}/../install_deps.log
GPERF_DIR=${DEPS_DIR}/gperftools-2.4
LIBUV_DIR=${DEPS_DIR}/libuv-v1.10.1

rm -f $LOG
mkdir -p ${DEPS_DIR}/build


platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
   platform='linux'
elif [[ "$unamestr" == 'FreeBSD' ]]; then
   platform='freebsd'
elif [[ "$unamestr" == 'Darwin' ]]; then
   platform='darwin'
fi



pushd $GPERF_DIR
touch $LOG
cp configure.orig configure
cp Makefile.in.orig Makefile.in
#make clean
echo "Echo building dependencies..." >> $LOG
./configure $CONFIG_OPTIONS --prefix=${DEPS_DIR}/build --enable-frame-pointers --with-pic 2>&1 >> $LOG || echo "Failed in configure for gperftools" >> $LOG
make -j4 2>&1 >> $LOG || echo "Failed to compile gperftools" >> $LOG
make install 2>&1 >> $LOG || echo "Failed to install gperftools to: $DEPS_DIR/build" >> $LOG
#echo "Error building gperftools-2.4" > $LOG
#make clean
rm -f $GPERF_DIR/Makefile
if [ -e "$DEPS_DIR/build/include/google/tcmalloc.h" ]; then
    echo "Successful build of depenencies" >> $LOG
    echo "ok"
    exit 0
else
    echo "Missing tcmalloc.h!!" >> $LOG
    echo "notok"
    exit 1
fi
rm -f Makefile.in
popd

echo "build libuv...."

pushd $LIBUV_DIR
if [[ "$platform" == 'darwin' ]]
	./gyp_uv.py -f xcode
	xcodebuild -ARCHS="x86_64" -project uv.xcodeproj -configuration Release -target All
else
	./gyp_uv.py -f make
	make -C out
fi
cp ./build/Release/libuv.a $DEPS_DIR/build/lib
popd