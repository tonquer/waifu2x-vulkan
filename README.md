# waifu2x-ncnn-vulkan-python
这是修改了waifu2x-ncnn-vulkan项目，导出pyd给python使用

## 在Python中使用
### 将生成的waifu.pyd放入代码目录或者python安装DLLs目录
```shell
import waifu2x
```
### 函数
```shell
模型索引modelIndex
waifu2x.MODEL_CUNET_NO_NOISE
waifu2x.MODEL_CUNET_NOISE3
waifu2x.MODEL_CUNET_NOISE3_TTA
waifu2x.MODEL_ANIME_STYLE_ART_RGB_NOISE3
waifu2x.MODEL_PHOTO_NOISE3
等等
命名规则
MODEL_模型名_NOISE降噪等级
MODEL_模型名_NOISE降噪等级_TTA
```
```shell
waifu2x.init()
# 初始化ncnn，只能执行一次
```
```shell
waifu2x.getGpuInfo()
# 返回一个gpu名的字符串列表，索引代表gpuId
```
```shell
waifu2x.initSet(gpuId: int, threadNum: int, model: str)
# 初始化设置参数，只能执行一次
# gpuId, -1为CPU，你选择的gpu
# threadNum，启动图片处理的线程数
# model，可选, 选择加载的model名字符串，如不传入将加载所有的models到内存中
```
```shell
waifu2x.add(data: bytes, modelIndex: int, backId: int, format: str, width: int, high: int, scale: float)
# 添加图片处理，需初始化后才能调用
# data, 图片的字节
# modelIndex, 选择的模型索引
# backId, 传入后，load函数会回传此Id
# format, 可选, 图片处理后转化成"jpg", "png"
# width, 可选, 图片放大后的宽度，和high一起使用
# high, 可选, 图片放大后的长度，和width一起使用
# scale, 可选, 可以不指定width和high，输入放大比例
```
```shell
waifu2x.load(block: int)
# 取出处理好的图片，需初始化后才能调用
# block, 0堵塞（不建议使用），非0 不堵塞
```
```shell
waifu2x.clear()
# 清空未处理的和已经处理的图片列表 ，需初始化后才能调用
```
```shell
waifu2x.stop()
# 停止图片转化，并且删除所有未处理的任务 ，需初始化后才能调用
```
### 示例
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
cd build -DPYTHON_INCLUDE_DIR=C:\Python37\include -DPYTHON_LIBRARY=C:\Python37\libs\python37.lib
cmake ../src
cmake --build . -j 4 --config Release
```
5. 成功后在build/Release目录，将waifu2x.dll改名waifu2x.pyd，即可使用
