# waifu2x-ncnn-vulkan-python
- This is modified [waifu2x-ncnn-vulkan](https://github.com/nihui/waifu2x-ncnn-vulkan), Export pyd and so files to Python
- Support Linux, Windows, MacOs
- Support JPG, PNG, BMP
- 
# Install
```shell
pip install waifu2x-vulkan -v
```

# Use
```shell
import waifu2x_vulkan

# init
waifu2x_vulkan.setDebug(True)
sts = waifu2x_vulkan.init()
assert sts==0
gpuList = waifu2x_vulkan.getGpuInfo()
print(gpuList)
sts = waifu2x_vulkan.initSet(gpuId=0, threadNum=2)
assert sts==0

# add picture ...
# waifu2x.add(...)

# load picture...
# newData, status, backId, tick = waifu2x.load(0)
```

## Example
- Please see [waifu2x-ncnn-vulkan-GUI](https://github.com/tonquer/waifu2x-ncnn-vulkan-GUI)
- Please see [test](https://github.com/tonquer/waifu2x-vulkan/blob/main/test/test.py) Example

## Build (Windows)
1. install CMake, Python, 7z
2. git clone
````shell
git clone https://github.com/tonquer/waifu2x-vulkan && cd waifu2x-vulkan
git submodule update --init --recursive
````
3. Install Python，Please check select  
````
Download debugging symbols  
Download debug binaries
````
4. run (Use PowerShell) , set python path
````shell
$Env:PYTHON_BIN='C:\Python37\python.exe'
.\build.ps1
````

```
* 部分用户可能需要安装 Vulkan 运行环境, 这取决于你的显卡驱动中是否包含 vulkan.1.dll;\
  如果编译后 waifu2x 无法使用, 请尝试安装: [最新的 Vulkan Runtime (来自官方)](https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-runtime.exe)
```

## Build (macOS)
1. 安装 [Xcode](https://apps.apple.com/us/app/xcode/id497799835?mt=12), [Homebrew](https://brew.sh/), CMake 和 Python
````shell
brew install cmake python@3.9
````
2. 克隆本仓库
````shell
git clone https://github.com/tonquer/waifu2x-vulkan && cd waifu2x-vulkan
git submodule update --init --recursive
````
3. 执行编译脚本
```shell
bash build_mac.sh
```
<!-- **Commented, bcs we now using the Homebrew Python3 instead**
* 若在 import waifu2x 时出现 Segfault, 需在编译之前 unlink 掉 Homebrew 安装的 Python 3.x:
````shell
brew unlink python@3.{8,9}
export PATH=$DEVELOPER_DIR/Library/Frameworks/Python3.framework/Versions/3.8/bin:$PATH
hash -r
````
* 在使用了 waifu2x 的项目中, 如果出现 pyinstaller 打包之后突然找不到依赖库的情况, 尝试进行:
````shell Library not loaded: @rpath/Python3.framework/Versions/3.7/Python3
install_name_tool -change @rpath/Python3.framework/Versions/3.7/Python3 @loader_path/Python3 waifu2x.so
````
-->
## Build (Linux)
1. 安装所需的软件包 (Ubuntu)
* P.S. 对于其他的发行版, 请自行寻找这些软件包所对应的名称, 然后使用软件包管理器手动安装它们
````shell
sudo apt-get update
sudo apt-get install --no-install-recommends --no-install-suggests \
                     python3.9 python3.9-dev build-essential cmake
````
2. 克隆本仓库
````shell
git clone https://github.com/tonquer/waifu2x-vulkan && cd waifu2x-vulkan
git submodule update --init --recursive
````
3. 执行编译脚本
```shell
bash build.sh
```
* 如果编译后 waifu2x 无法使用, 请尝试安装: `libvulkan1` 软件包
