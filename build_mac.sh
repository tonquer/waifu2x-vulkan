#!/bin/bash

export DEVELOPER_DIR=/Applications/Xcode_12.2.app/Contents/Developer

# brew install protobuf sdl

# OpemMP
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/openmp-11.0.0.src.tar.xz
tar -xf openmp-11.0.0.src.tar.xz
cd openmp-11.0.0.src
sed -i'' -e '/.size __kmp_unnamed_critical_addr/d' runtime/src/z_Linux_asm.S
sed -i'' -e 's/__kmp_unnamed_critical_addr/___kmp_unnamed_critical_addr/g' runtime/src/z_Linux_asm.S

# OpenMP (x86_64 arm64)
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=install \
      -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
      -DLIBOMP_ENABLE_SHARED=OFF \
      -DLIBOMP_OMPT_SUPPORT=OFF \
      -DLIBOMP_USE_HWLOC=OFF \
      ..
cmake --build . -j 2
cmake --build . --target install

# OpenMP Xcode macOS SDK
sudo cp install/include/* $DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
sudo cp install/lib/libomp.a $DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib

# Vulkan SDK
cd ../..
wget https://sdk.lunarg.com/sdk/download/1.2.162.0/mac/vulkansdk-macos-1.2.162.0.dmg?Human=true -O vulkansdk-macos-1.2.162.0.dmg
hdiutil attach vulkansdk-macos-1.2.162.0.dmg
cp -r /Volumes/vulkansdk-macos-1.2.162.0 .
rm -rf vulkansdk-macos-1.2.162.0/Applications
find vulkansdk-macos-1.2.162.0 -type f | grep -v -E 'vulkan|glslang|MoltenVK' | xargs rm
hdiutil detach /Volumes/vulkansdk-macos-1.2.162.0
export VULKAN_SDK=`pwd`/vulkansdk-macos-1.2.162.0/macOS

# Python x86_64
mkdir build-x86_64 && cd build-x86_64
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DUSE_STATIC_MOLTENVK=ON \
      -DPYTHON_EXECUTABLE=$DEVELOPER_DIR/Library/Frameworks/Python3.framework/Versions/3.8/bin/python3 \
      -DPYBIND11_FINDPYTHON=OFF \
      -DPYBIND11_PYTHON_VERSION=3.8.2 \
      -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
      -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DOpenMP_C_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_CXX_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_C_LIB_NAMES="libomp"\
      -DOpenMP_CXX_LIB_NAMES="libomp" \
      -DOpenMP_libomp_LIBRARY="$DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/libomp.a" \
      -DVulkan_INCLUDE_DIR=$VULKAN_SDK/../MoltenVK/include \
      -DVulkan_LIBRARY=$VULKAN_SDK/../MoltenVK/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a \
      ../src
cmake --build . -j 2

# Python arm64
cd ..
mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DUSE_STATIC_MOLTENVK=ON \
      -DPYTHON_EXECUTABLE=$DEVELOPER_DIR/Library/Frameworks/Python3.framework/Versions/3.8/bin/python3 \
      -DPYBIND11_FINDPYTHON=OFF \
      -DPYBIND11_PYTHON_VERSION=3.8.2 \
      -DCMAKE_CROSSCOMPILING=ON \
      -DCMAKE_SYSTEM_PROCESSOR=arm64 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DOpenMP_C_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_CXX_FLAGS="-Xclang -fopenmp" \
      -DOpenMP_C_LIB_NAMES="libomp" \
      -DOpenMP_CXX_LIB_NAMES="libomp" \
      -DOpenMP_libomp_LIBRARY="$DEVELOPER_DIR/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/libomp.a" \
      -DVulkan_INCLUDE_DIR=$VULKAN_SDK/../MoltenVK/include \
      -DVulkan_LIBRARY=$VULKAN_SDK/../MoltenVK/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a \
      ../src
cmake --build . -j 2

# Python (x86_64 arm64)
cd ..
lipo -create build-x86_64/libwaifu2x.dylib build-arm64/libwaifu2x.dylib -o waifu2x.so

strip -x waifu2x.so
