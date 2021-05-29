LIB_NAME=waifu2x
TAG_NAME=$(git describe --abbrev=0 --tags)
HEAD_SHA_SHORT=$(git rev-parse --short HEAD)
PACKAGE_PREFIX=${LIB_NAME}-${TAG_NAME}_${HEAD_SHA_SHORT}
PACKAGENAME=${PACKAGE_PREFIX}-py39-ubuntu

# Vulkan SDK
wget https://sdk.lunarg.com/sdk/download/1.2.162.0/linux/vulkansdk-linux-x86_64-1.2.162.0.tar.gz?Human=true \
     -O vulkansdk-linux-x86_64-1.2.162.0.tar.gz
tar -xf vulkansdk-linux-x86_64-1.2.162.0.tar.gz
rm -rf 1.2.162.0/source 1.2.162.0/samples
find 1.2.162.0 -type f | grep -v -E 'vulkan|glslang' | xargs rm

# Python x86_64
export VULKAN_SDK=`pwd`/1.2.162.0/x86_64
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DPYTHON_EXECUTABLE=/usr/bin/python3.9 \
      -DPYBIND11_FINDPYTHON=OFF \
      -DPYBIND11_PYTHON_VERSION="3.9.5" \
      ../src
cmake --build . -j 2
cp libwaifu2x.so waifu2x.so
strip -x waifu2x.so

# Package
cd ..
mkdir -p $PACKAGENAME
cp README.md LICENSE $PACKAGENAME
cp build/waifu2x.so $PACKAGENAME
cp -r models test $PACKAGENAME
