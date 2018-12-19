cfg() {
  : ${build:=`gcc -dumpmachine`}

  if [ -z "$host" ]; then
    host=$build
#    case "$host" in
#      x86_64-w64-mingw32) host="$host" builddir=build/mingw64 prefix=/mingw64 ;;
#      i686-w64-mingw32) host="$host" builddir=build/mingw32 prefix=/mingw32 ;;
#      x86_64-pc-*) host="$host" builddir=build/${host#*-pc-}64 prefix=/usr ;;
#      i686-pc-*) host="$host" builddir=build/${host#*-pc-}32 prefix=/usr ;;
#    esac
  fi
  : ${prefix:=/usr}
  : ${libdir:=$prefix/lib}
  [ -d "$libdir/$host" ] && libdir=$libdir/$host

  if [ -e "$TOOLCHAIN" ]; then
    cmakebuild=$(basename "$TOOLCHAIN" .cmake)
    cmakebuild=${cmakebuild%.toolchain}
    cmakebuild=cmake-${cmakebuild#toolchain-}
    : ${builddir=build/$cmakebuild}
  else
   : ${builddir=build/cmake-$host}
  fi

  case $(uname -o) in
   # MSys|MSYS|Msys) SYSTEM="MSYS" ;;
    *) SYSTEM="Unix" ;;
  esac

  case "$STATIC:$TYPE" in
    YES:*|yes:*|y:*|1:*|ON:*|on:* | *:*[Ss]tatic*) set -- "$@" \
      -DBUILD_SHARED_LIBS=OFF \
      -DENABLE_PIC=OFF ;;
  esac
  : ${generator:="Unix Makefiles"}

 (mkdir -p $builddir
  : ${relsrcdir=`realpath --relative-to "$builddir" .`}
  set -x
  cd $builddir
  ${CMAKE:-cmake} -Wno-dev \
    -DCMAKE_INSTALL_PREFIX="${prefix-/usr}" \
    -G "$generator" \
    ${VERBOSE+:-DCMAKE_VERBOSE_MAKEFILE=TRUE} \
    -DCMAKE_BUILD_TYPE="${TYPE:-RelWithDebInfo}" \
    ${CC:+-DCMAKE_C_COMPILER="$CC"} \
    ${CXX:+-DCMAKE_CXX_COMPILER="$CXX"} \
    ${PKG_CONFIG:+-DPKG_CONFIG_EXECUTABLE="$PKG_CONFIG"} \
    ${TOOLCHAIN:+-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"} \
    ${CC:+-DCMAKE_C_COMPILER="$CC"} \
    ${CXX:+-DCMAKE_CXX_COMPILER="$CXX"} \
    -DCMAKE_{C,CXX}_FLAGS_DEBUG="-g -ggdb3" \
    -DCMAKE_{C,CXX}_FLAGS_RELWITHDEBINFO="-O2 -g -ggdb3 -DNDEBUG" \
    ${MAKE:+-DCMAKE_MAKE_PROGRAM="$MAKE"} \
    "$@" \
    $relsrcdir 2>&1 ) |tee "${builddir##*/}.log"
}

cfg-android () 
{
  (: ${builddir=build/android}
    cfg \
  -DCMAKE_INSTALL_PREFIX=/opt/arm-linux-androideabi/sysroot/usr \
  -DCMAKE_VERBOSE_MAKEFILE=TRUE \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN:-/opt/android-cmake/android.cmake} \
  -DANDROID_NATIVE_API_LEVEL=21 \
  -DPKG_CONFIG_EXECUTABLE=arm-linux-androideabi-pkg-config \
  -DCMAKE_PREFIX_PATH=/opt/arm-linux-androideabi/sysroot/usr \
  -DCMAKE_MAKE_PROGRAM=/usr/bin/make \
    "$@"
    )
}

cfg-diet() {
 (build=$(${CC:-gcc} -dumpmachine)
  host=${build/-gnu/-dietlibc}
  : ${builddir=build/$host}
  : ${prefix=/opt/diet}
  : ${libdir=/opt/diet/lib-${host%%-*}}
  : ${bindir=/opt/diet/bin-${host%%-*}}
  
  CC="diet-gcc" \
  PKG_CONFIG="$host-pkg-config" \
  LIBS="${LIBS:+$LIBS }-liconv -lpthread" \
  cfg \
    -DSHARED_LIBS=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}

cfg-musl() {
 (build=$(${CC:-gcc} -dumpmachine)
  host=${build/-gnu/-musl}
  builddir=build/$host
  prefix=/usr
  includedir=/usr/include/$host
  libdir=/usr/lib/$host
  bindir=/usr/bin/$host
  
  CC=musl-gcc \
  PKG_CONFIG=musl-pkg-config \
  cfg \
    -DSHARED_LIBS=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}

cfg-mingw() {
 (build=$(gcc -dumpmachine)
  host=${build%%-*}-w64-mingw32
  prefix=/usr/$host/sys-root/mingw
  TOOLCHAIN=/usr/x86_64-w64-mingw32/sys-root/toolchain-mingw64.cmake
  
  builddir=build/$host \
  bindir=$prefix/bin \
  libdir=$prefix/lib \
  cfg \
    "$@")
}
cfg-termux() 
{
  (builddir=build/termux
    cfg \
  -DCMAKE_INSTALL_PREFIX=/data/data/com.termux/files/usr \
  -DCMAKE_VERBOSE_MAKEFILE=TRUE \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN:-/opt/android-cmake/android.cmake} \
  -DANDROID_NATIVE_API_LEVEL=21 \
  -DPKG_CONFIG_EXECUTABLE=arm-linux-androideabi-pkg-config \
  -DCMAKE_PREFIX_PATH=/data/data/com.termux/files/usr \
  -DCMAKE_MAKE_PROGRAM=/usr/bin/make \
   -DCMAKE_MODULE_PATH="/data/data/com.termux/files/usr/lib/cmake" \
   "$@"
    )
}

cfg-msys32() { 
    cfg \
      -DCMAKE_EXE_LINKER_FLAGS="-static" \
      -DBZIP2_LIBRAR{Y,IES}="$prefix/lib/libbz2.a" \
      -DZLIB_LIBRARY="$prefix/lib/libz.a" \
      -DLIBLZMA_LIBRARY="$prefix/lib/liblzma.a" \
      -DOPENSSL_CRYPTO_LIBRARY="$prefix/lib/libcrypto.a" \
      -DOPENSSL_SSL_LIBRARY="$prefix/lib/libssl.a" \
      -DUSE_SELECT=ON \
      "$@"
}
