LIB_NAME=waifu2x-vulkan
TAG_NAME=$(git describe --abbrev=0 --tags)
HEAD_SHA_SHORT=$(git rev-parse --short HEAD)
PACKAGE_PREFIX=${LIB_NAME}-${TAG_NAME}_${HEAD_SHA_SHORT}
PACKAGENAME=${PACKAGE_PREFIX}-ubuntu
oldPath=`pwd`
# Vulkan SDK
# if [ ! -d "1.2.162.0" ];then
#       wget 'https://sdk.lunarg.com/sdk/download/1.2.162.0/linux/vulkansdk-linux-x86_64-1.2.162.0.tar.gz?Human=true' \
#       -O vulkansdk-linux-x86_64-1.2.162.0.tar.gz
#       tar -xf vulkansdk-linux-x86_64-1.2.162.0.tar.gz
#       rm -rf 1.2.162.0/source 1.2.162.0/samples
#       find 1.2.162.0 -type f | grep -v -E 'vulkan|glslang' | xargs rm
# fi
# export VULKAN_SDK=`pwd`/1.2.162.0/x86_64
mkdir -p build && cd build

# Python x86_64
if [ ! -n "$PYTHON_BIN" ]; then
      PYTHON_BIN=`which python3`
fi
PYTHON_DIR=`dirname $PYTHON_BIN`/../

VERSION=`${PYTHON_BIN} -V 2>&1 | cut -d " " -f 2`
VERSION_INFO=${VERSION:0:3}

PYTHON_LIBRARIES=`find $PYTHON_DIR -name "libpython${VERSION_INFO}*.a"|tail -1`
PYTHON_INCLUDE=`find $PYTHON_DIR -name "Python.h"|tail -1`
PYTHON_INCLUDE_DIRS=`dirname $PYTHON_INCLUDE`

echo $VERSION
echo $PYTHON_BIN
echo $PYTHON_LIBRARIES
echo $PYTHON_INCLUDE_DIRS

cmake -DCMAKE_BUILD_TYPE=Release \
      -DVulkan_LIBRARY="../VulkanSDK/linux/libvulkan.so" \
      -DVulkan_INCLUDE_DIR="../VulkanSDK/Include" \
      -DDCMAKE_VERBOSE_MAKEFILE=On \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
      -DPYTHON_INCLUDE_DIRS=$PYTHON_INCLUDE_DIRS \
      ../src
cmake --build .
cp libwaifu2x_vulkan.so waifu2x_vulkan.so
strip -x waifu2x_vulkan.so

# Package
cd $oldPath
mkdir -p $PACKAGENAME
cp README.md LICENSE $PACKAGENAME
cp build/waifu2x_vulkan.so $PACKAGENAME
cp -r waifu2x_vulkan/models test $PACKAGENAME
