export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
LIB_NAME=waifu2x-ncnn-vulkan-python
TAG_NAME=$(git describe --abbrev=0 --tags)
HEAD_SHA_SHORT=$(git rev-parse --short HEAD)
PACKAGE_PREFIX=${LIB_NAME}-${TAG_NAME}_${HEAD_SHA_SHORT}
PACKAGENAME=${PACKAGE_PREFIX}-py37-macos

# OpemMP
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
cmake --build . -j 2
cmake --build . --target install

# OpenMP Xcode macOS SDK
sudo cp install/include/* \
        $DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
sudo cp install/lib/libomp.a \
        $DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib

# Vulkan SDK
cd ../..
wget 'https://sdk.lunarg.com/sdk/download/1.2.162.0/mac/vulkansdk-macos-1.2.162.0.dmg?Human=true' \
     -O vulkansdk-macos-1.2.162.0.dmg
hdiutil attach vulkansdk-macos-1.2.162.0.dmg
cp -r /Volumes/vulkansdk-macos-1.2.162.0 .
rm -rf vulkansdk-macos-1.2.162.0/Applications
find vulkansdk-macos-1.2.162.0 -type f | grep -v -E 'vulkan|glslang|MoltenVK' | xargs rm
hdiutil detach /Volumes/vulkansdk-macos-1.2.162.0
export VULKAN_SDK=`pwd`/vulkansdk-macos-1.2.162.0/macOS
PythonDir=`which python3`
PythonDir=${PythonDir%/*}/..
VERSION=`python3 -V 2>&1 | cut -d " " -f 2`
PyVer=${Version:0:3}
LibDir=`find  $PythonDir -name "libpython*.a"|grep $PyVer|tail -1`
IncludeDir=`find  $PythonDir -name "Python.h"|tail -1`
IncludeDir=${IncludeDir%/*}

# Python
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DUSE_STATIC_MOLTENVK=ON \
      -DPYTHON_INCLUDE_DIRS=${IncludeDir} \
      -DPYTHON_LIBRARY=${LibDir} \
      -DOpenMP_C_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_CXX_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_C_LIB_NAMES="libomp"\
      -DOpenMP_CXX_LIB_NAMES="libomp" \
      -DOpenMP_libomp_LIBRARY="$DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/libomp.a" \
      -DVulkan_INCLUDE_DIR=$VULKAN_SDK/../MoltenVK/include \
      -DVulkan_LIBRARY=$VULKAN_SDK/../MoltenVK/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a \
      ../src
cmake --build . -j 2
cp libwaifu2x.dylib waifu2x.so
strip -x waifu2x.so

# Package
cd ..
mkdir -p $PACKAGENAME
cp README.md LICENSE $PACKAGENAME
cp build/waifu2x.so $PACKAGENAME
cp -r models test $PACKAGENAME
