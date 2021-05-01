# waifu2x-ncnn-vulkan-python
这是修改了waifu2x-ncnn-vulkan项目，导出pyd和so文件给python使用
支持Linux和Windows

## 在Python中使用
将生成的waifu.pyd放入代码目录或者python安装DLLs目录
```shell
import waifu2x
```

## 示例
使用该项目和Qt实现的waifu2x,GUI小工具，[waifu2x-GUI]（https://github.com/tonquer/waifu2x-ncnn-vulkan-GUI）
请看test中的示例

## 编译（Windows）
1. 下载安装VulkanSDK https://vulkan.lunarg.com/sdk/home
2. 安装Python3.7+，安装时在Advanced Options需要勾选Download debugging symbols和Download debug binaries
3. git拷贝仓库
```shell
git clone https://github.com/tonquer/waifu2x-ncnn-vulkan-python.git
cd waifu2x-ncnn-vulkan-python
git submodule update --init --recursive
```
4. 打开目录waifu2x-ncnn-vulkan-python，Cmake编译，指定Python目录
```shell
mkdir build
cd build
cmake ../src -DPYTHON_INCLUDE_DIR=C:\Python37\include -DPYTHON_LIBRARY=C:\Python37\libs\python37.lib
cmake --build .. -j 4 --config Release
```
5. 成功后在build/Release目录，将waifu2x.dll改名waifu2x.pyd，即可使用

## 编译（Linux）（由于cmake错误没解决，只能手动编译）
1. 安装依赖
  ``` sheel
  sudo apt install build-essential git cmake libprotobuf-dev protobuf-compiler libvulkan-dev vulkan-utils libopencv-dev
  sudo apt install libvulkan1 mesa-vulkan-drivers vulkan-utils
  ```
2. 下载Python，以3.7.9为列，其他python3版本也可，wget https://www.python.org/ftp/python/3.7.9/Python-3.7.9.tar.xz
3. 解压编译python，
  ```shell
  tar xvf Python-3.7.9.tar.xz && cd Python-3.7.9/
  ./configure --enable-shared --enable-unicode=ucs4 --enable-optimizations
  make
  ```
4. git克隆代码，编译ncnn和libweb，如有报错请解决错误再往下编译
  ```shell
  git clone https://github.com/tonquer/waifu2x-ncnn-vulkan-python.git
  cd waifu2x-ncnn-vulkan-python
  git submodule update --init --recursive
  sh build.sh
  ```
5. 编译waifu2x-ncnn-vulkan-python, 编辑build_waifu2x.sh，把Python目录改为，你编译好的python目录
  ```shell
  vim build_waifu2x  
  sh build_waifu2x.sh
  ```
6. 成功后会生成waifu2x.so, 把该文件放入到你的项目目录
  ```python 
  import waifu2x
  ```
