export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
LIB_NAME=waifu2x-vulkan
TAG_NAME=$(git describe --abbrev=0 --tags)
HEAD_SHA_SHORT=$(git rev-parse --short HEAD)
PACKAGE_PREFIX=${LIB_NAME}-${TAG_NAME}_${HEAD_SHA_SHORT}
PACKAGENAME=${PACKAGE_PREFIX}-macos
oldPath=`pwd`
# OpemMP
if [ ! -d "openmp-11.0.0.src" ];then
      wget 'https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/openmp-11.0.0.src.tar.xz'
      tar -xf openmp-11.0.0.src.tar.xz
      cd openmp-11.0.0.src
      sed -i'' -e '/.size __kmp_unnamed_critical_addr/d' runtime/src/z_Linux_asm.S
      sed -i'' -e 's/__kmp_unnamed_critical_addr/___kmp_unnamed_critical_addr/g' runtime/src/z_Linux_asm.S

      # OpenMP
      mkdir -p build && cd build
      cmake -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=install \
            -DLIBOMP_ENABLE_SHARED=OFF \
            -DLIBOMP_OMPT_SUPPORT=OFF \
            -DLIBOMP_USE_HWLOC=OFF \
            ..
      cmake --build .
      cmake --build . --target install

      # OpenMP Xcode macOS SDK
      sudo cp install/include/* \
            $DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
      sudo cp install/lib/libomp.a \
            $DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib

      cd ../..
fi

# Vulkan SDK
if [ ! -d "vulkansdk-macos-1.2.162.0" ];then
      wget 'https://sdk.lunarg.com/sdk/download/1.2.162.0/mac/vulkansdk-macos-1.2.162.0.dmg?Human=true' \
      -O vulkansdk-macos-1.2.162.0.dmg
      hdiutil attach vulkansdk-macos-1.2.162.0.dmg      
      cp -r /Volumes/vulkansdk-macos-1.2.162.0 .
      rm -rf vulkansdk-macos-1.2.162.0/Applications
      find vulkansdk-macos-1.2.162.0 -type f | grep -v -E 'vulkan|glslang|MoltenVK' | xargs rm
      hdiutil detach /Volumes/vulkansdk-macos-1.2.162.0
fi

export VULKAN_SDK=`pwd`/vulkansdk-macos-1.2.162.0/macOS

# Python x86_64
if [ ! -n "$PYTHON_BIN" ]; then
      PYTHON_BIN=`which python3`
fi
PYTHON_DIR=`dirname $PYTHON_BIN`/../

VERSION=`${PYTHON_BIN} -V 2>&1 | cut -d " " -f 2`
VERSION_INFO=${VERSION:0:3}

PYTHON_LIBRARIES=`find $PYTHON_DIR -name "libpython${VERSION_INFO}*.dylib"|tail -1`
PYTHON_INCLUDE=`find $PYTHON_DIR -name "Python.h"|tail -1`
PYTHON_INCLUDE_DIRS=`dirname $PYTHON_INCLUDE`

echo $VERSION
echo $PYTHON_BIN
echo $PYTHON_LIBRARIES
echo $PYTHON_INCLUDE_DIRS

# Python
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DUSE_STATIC_MOLTENVK=ON \
      -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
      -DPYTHON_INCLUDE_DIRS=$PYTHON_INCLUDE_DIRS \
      -DOpenMP_C_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_CXX_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_C_LIB_NAMES="libomp"\
      -DOpenMP_CXX_LIB_NAMES="libomp" \
      -DOpenMP_libomp_LIBRARY="$DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/libomp.a" \
      -DVulkan_INCLUDE_DIR=$VULKAN_SDK/../MoltenVK/include \
      -DVulkan_LIBRARY=$VULKAN_SDK/../MoltenVK/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a \
      ../src
cmake --build .
cp libwaifu2x_vulkan.dylib waifu2x_vulkan.so
strip -x waifu2x_vulkan.so

# Package
cd $oldPath
mkdir -p $PACKAGENAME
cp README.md LICENSE $PACKAGENAME
cp build/waifu2x_vulkan.so $PACKAGENAME
cp -r waifu2x_vulkan/models test $PACKAGENAME
