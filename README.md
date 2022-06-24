# waifu2x-ncnn-vulkan-python
- This is modified [waifu2x-ncnn-vulkan](https://github.com/nihui/waifu2x-ncnn-vulkan), Export pyd and so files to Python
- Support Linux, Windows, MacOs
- Support import JPG, PNG, BMP, GIF, WEBP, Animated WEBP 
- Support export JPG, PNG, BMP, WEBP, Animated WEBP
# Install
```shell
pip install waifu2x-vulkan -v
```

# Use
```shell
from waifu2x_vulkan import waifu2x_vulkan

# init
waifu2x_vulkan.setDebug(True)
sts = waifu2x_vulkan.init()
if sts < 0:
    # cpu model
    isCpuModel = True
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

## Build
```shell
pip install wheel
python setup.py bdist_wheel
```