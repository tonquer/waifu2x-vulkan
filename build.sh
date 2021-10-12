LIB_NAME=waifu2x-vulkan
TAG_NAME=$(git describe --abbrev=0 --tags)
HEAD_SHA_SHORT=$(git rev-parse --short HEAD)
PACKAGE_PREFIX=${LIB_NAME}-${TAG_NAME}_${HEAD_SHA_SHORT}
PACKAGENAME1=${PACKAGE_PREFIX}-ubuntu
$oldPath=`pwd`
# Vulkan SDK
if [ ! -d "1.2.162.0" ];then
      wget 'https://sdk.lunarg.com/sdk/download/1.2.162.0/linux/vulkansdk-linux-x86_64-1.2.162.0.tar.gz?Human=true' \
      -O vulkansdk-linux-x86_64-1.2.162.0.tar.gz
      tar -xf vulkansdk-linux-x86_64-1.2.162.0.tar.gz
      rm -rf 1.2.162.0/source 1.2.162.0/samples
      find 1.2.162.0 -type f | grep -v -E 'vulkan|glslang' | xargs rm
fi

# Python x86_64
if [ ! -n "$PYTHON_BIN" ]; then
      $PYTHON_BIN="${pythonLocation}/bin/python3"
fi

if [ ! -n "$BUILD_PATH" ]; then
      $BUILD_PATH="build"
fi

export VULKAN_SDK=`pwd`/1.2.162.0/x86_64
mkdir -p $BUILD_PATH && cd $BUILD_PATH

VERSION=`${PYTHON_BIN} -V 2>&1 | cut -d " " -f 2`
echo $VERSION
echo $PYTHON_BIN
cmake -DCMAKE_BUILD_TYPE=Release \
      -DNCNN_VULKAN=ON \
      -DNCNN_BUILD_TOOLS=OFF \
      -DNCNN_BUILD_EXAMPLES=OFF \
      -DPYTHON_EXECUTABLE=${PYTHON_BIN}  \
      -DPYBIND11_FINDPYTHON=OFF \
      ../src
cmake --build . -j 2
cp libwaifu2x_vulkan.so waifu2x_vulkan.so
strip -x waifu2x_vulkan.so

# Package
PACKAGENAME=${PACKAGENAME1}-py${VERSION}
cd $oldPath
mkdir -p $PACKAGENAME
cp README.md LICENSE $PACKAGENAME
cp $BUILD_PATH/waifu2x_vulkan.so $PACKAGENAME
cp -r models test $PACKAGENAME
