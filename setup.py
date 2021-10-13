import setuptools, sys, os
from shutil import copyfile, copytree

long_description = \
"""
# waifu2x-ncnn-vulkan-python
- This is modified [waifu2x-ncnn-vulkan](https://github.com/nihui/waifu2x-ncnn-vulkan), Export pyd and so files to Python
- Support Linux, Windows, MacOs
- Support JPG, PNG, BMP
- Need support vulkan gpu

# Install
```shell
pip install waifu2x_vulkan
```

# Use
```shell
import waifu2x_vulkan

# init
sts = waifu2x_vulkan.init()
assert sts==0
gpuList = waifu2x_vulkan.getGpuInfo()
print(gpuList)
sts = waifu2x_vulkan.initSet(gpuId=0, threadNum=2)
assert sts==0

# Model List:
#'MODEL_ANIME_STYLE_ART_RGB_NOISE0', 'MODEL_ANIME_STYLE_ART_RGB_NOISE0_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NOISE1', 'MODEL_ANIME_STYLE_ART_RGB_NOISE1_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NOISE2', 'MODEL_ANIME_STYLE_ART_RGB_NOISE2_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NOISE3', 'MODEL_ANIME_STYLE_ART_RGB_NOISE3_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NO_NOISE', 'MODEL_ANIME_STYLE_ART_RGB_NO_NOISE_TTA', 'MODEL_CUNET_NOISE0', 'MODEL_CUNET_NOISE0_TTA', 'MODEL_CUNET_NOISE1', 'MODEL_CUNET_NOISE1_TTA', 'MODEL_CUNET_NOISE2', 'MODEL_CUNET_NOISE2_TTA', 'MODEL_CUNET_NOISE3', 'MODEL_CUNET_NOISE3_TTA', 'MODEL_CUNET_NO_NOISE', 'MODEL_CUNET_NO_NOISE_TTA', 'MODEL_CUNET_NO_SCALE_NOISE0', 'MODEL_CUNET_NO_SCALE_NOISE0_TTA', 'MODEL_CUNET_NO_SCALE_NOISE1', 'MODEL_CUNET_NO_SCALE_NOISE1_TTA', 'MODEL_CUNET_NO_SCALE_NOISE2', 'MODEL_CUNET_NO_SCALE_NOISE2_TTA', 'MODEL_CUNET_NO_SCALE_NOISE3', 'MODEL_CUNET_NO_SCALE_NOISE3_TTA', 'MODEL_CUNET_NO_SCALE_NO_NOISE', 'MODEL_CUNET_NO_SCALE_NO_NOISE_TTA', 'MODEL_PHOTO_NOISE0', 'MODEL_PHOTO_NOISE0_TTA', 'MODEL_PHOTO_NOISE1', 'MODEL_PHOTO_NOISE1_TTA', 'MODEL_PHOTO_NOISE2', 'MODEL_PHOTO_NOISE2_TTA', 'MODEL_PHOTO_NOISE3', 'MODEL_PHOTO_NOISE3_TTA', 'MODEL_PHOTO_NO_NOISE', 'MODEL_PHOTO_NO_NOISE_TTA'

# add picture ...
# waifu2x.add(data=imgData, modelIndex=waifu2x_vulkan.MODEL_ANIME_STYLE_ART_RGB_NOISE0, backId=0, scale=2.5)
# waifu2x.add(data=imgData, modelIndex=waifu2x_vulkan.MODEL_ANIME_STYLE_ART_RGB_NOISE0, backId=0, format="png", width=1000, high=1000)

# load picture...
# newData, status, backId, tick = waifu2x.load(0)
```

"""
Version = "1.0.3"
Plat = sys.platform
for i in sys.argv:
    if "win" in i.lower():
        Plat = "win32"
    elif "macos" in i.lower():
        Plat = "darwin"
    elif "linux" in i.lower():
        Plat = "linux"

if os.path.exists("waifu2x_vulkan/waifu2x_vulkan.so"):
    os.remove("waifu2x_vulkan/waifu2x_vulkan.so")

if os.path.exists("waifu2x_vulkan/waifu2x_vulkan.pyd"):
    os.remove("waifu2x_vulkan/waifu2x_vulkan.pyd")

if Plat in ["win32", "win64"]:
    src = "lib/{}/linux/waifu2x_vulkan.pyd".format(Version)
    dest = "waifu2x_vulkan/waifu2x_vulkan.pyd"
elif Plat in ["darwin"]:
    src = "lib/{}/linux/waifu2x_vulkan.so".format(Version)
    dest = "waifu2x_vulkan/waifu2x_vulkan.so"
else:
    src = "lib/{}/linux/waifu2x_vulkan.so".format(Version)
    dest = "waifu2x_vulkan/waifu2x_vulkan.so"

copyfile(src, dest)
copytree("models", "waifu2x_vulkan/models")

setuptools.setup(
    name="waifu2x-vulkan",
    version=Version,
    author="tonquer",
    author_email="tonquer@outlook.com",
    description="A waifu2x vulkan tool",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/tonquer/waifu2x-vulkan",
    packages=setuptools.find_packages(),
    install_requires=[],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: POSIX :: Linux"
        "Operating System :: Microsoft :: Windows"
        "Operating System :: MacOS"
        "License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)",
    ],
    python_requires = ">=3.6",
    include_package_data=True,
)