cfg() { 
 (: ${build:=`gcc -dumpmachine`}
  [ -n "$build" ] && build=${build//-pc-/-}

  : ${host:=$build}
  : ${prefix:=/usr}
  : ${libdir:=$prefix/lib}
  [ -d "$libdir/$host" ] && libdir=$libdir/$host : ${builddir:=build/$host}

  case $(uname -o) in
    MSys|MSYS|Msys) SYSTEM="MSYS" ;;
    *) SYSTEM="Unix" ;;
  esac

  case "$STATIC:$TYPE" in
    YES:*|yes:*|y:*|1:*|ON:*|on:* | *:*[Ss]tatic*) set -- "$@" \
      -DBUILD_SHARED_LIBS=OFF \
      -DENABLE_PIC=OFF ;;
  esac
   case "$PYTHON" in
     NO|no|n|0|OFF|off) set -- "$@" \
        -DBUILD_opencv_python2=OFF \
        -DBUILD_opencv_python3=OFF \
        -DBUILD_opencv_python_bindings_generator=OFF \
        -DENABLE_PYLINT=OFF \
        -DINSTALL_PYTHON_EXAMPLES=OFF \
        -DOPENCV_FORCE_PYTHON_LIBS=OFF ;;
  esac
  case "$GTK" in
    2) GTK2="ON" ;;
    3) GTK3="ON" ;;
  esac

 [ -n "$CC" ] && { test -e "$CC"  || CC=$(which "$CC"); }
 [ -n "$CXX" ] && { test -e "$CXX"  || CXX=$(which "$CXX"); }

 (mkdir -p $builddir
  relsrcdir=$(/usr/bin/realpath --relative-to "$(realpath "$builddir")" "$(realpath "${PWD:-$(pwd)}")")
  set -x
  cd $builddir
  cmake \
  -Wno-dev \
    -DCMAKE_INSTALL_PREFIX="${prefix-/usr}" \
    -G "${SYSTEM:-Unix} Makefiles" \
    ${VERBOSE+:-DCMAKE_VERBOSE_MAKEFILE=TRUE} \
    -DCMAKE_BUILD_TYPE="${TYPE:-RelWithDebInfo}" \
    ${CC:+-DCMAKE_C_COMPILER="$CC"} \
    ${CXX:+-DCMAKE_CXX_COMPILER="$CXX"} \
    ${PKG_CONFIG:+-DCMAKE_PKG_CONFIG_EXECUTABLE="$PKG_CONFIG"} \
    ${TOOLCHAIN:+-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"} \
    ${CC:+-DCMAKE_C_COMPILER="$CC"} \
    ${CXX:+-DCMAKE_CXX_COMPILER="$CXX"} \
    -DCMAKE_{C,CXX}_FLAGS_DEBUG="-g -ggdb3" \
    -DCMAKE_{C,CXX}_FLAGS_RELWITHDEBINFO="-O2 -g -ggdb3 -DNDEBUG" \
    "$@" \
    ../.. 2>&1 ) |tee "${builddir##*/}.log")
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
   -DCMAKE_MODULE_PATH="/opt/OpenCV-3.4.1-android-sdk/sdk/native/jni/abi-armeabi-v7a" \
   -DOpenCV_DIR="/opt/OpenCV-3.4.1-android-sdk/sdk/native/jni/abi-armeabi-v7a" \
   "$@"
    )
}

diet-cfg() {
 (build=$(${CC:-gcc} -dumpmachine)
  host=${build/-gnu/-dietlibc}
  builddir=build/$host
  prefix=/opt/diet
  libdir=/opt/diet/lib-${host%%-*}
  bindir=/opt/diet/bin-${host%%-*}
  
  CC="diet-gcc" \
  PKG_CONFIG="$host-pkg-config" \
  LIBS="${LIBS:+$LIBS }-liconv -lpthread" \
  cfg \
    -DSHARED_LIBS=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}

musl-cfg() {
 (build=$(${CC:-gcc} -dumpmachine)
  host=${build/-gnu/-musl}
  host=${host/-pc-/-}
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

mingw-cfg() {
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
