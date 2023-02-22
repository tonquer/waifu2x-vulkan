import setuptools, sys, os
from shutil import copyfile, copytree, rmtree
from setuptools.command.build_ext import build_ext
import subprocess
from distutils.core import Extension

long_description = \
"""
# waifu2x-ncnn-vulkan-python
- This is modified [waifu2x-ncnn-vulkan](https://github.com/nihui/waifu2x-ncnn-vulkan), Export pyd and so files to Python
- Support Linux, Windows, MacOs
- Support import JPG, PNG, BMP, GIF, WEBP, Animated WEBP, APNG
- Support export JPG, PNG, BMP, WEBP, Animated WEBP, APNG
- Support vulkan gpu and cpu

# Install
```shell
pip install waifu2x-vulkan
```

# Use
```shell
from waifu2x_vulkan import waifu2x_vulkan

# init
sts = waifu2x_vulkan.init()
print("init, code:{}".format(str(sts)))
isCpuModel = False
if sts < 0:
    # cpu model
    isCpuModel = True

gpuList = waifu2x_vulkan.getGpuInfo()
print(gpuList)
sts = waifu2x_vulkan.initSet(gpuId=0)
print("init set, code:{}".format(str(sts)))

# Model List:
#'MODEL_ANIME_STYLE_ART_RGB_NOISE0', 'MODEL_ANIME_STYLE_ART_RGB_NOISE0_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NOISE1', 'MODEL_ANIME_STYLE_ART_RGB_NOISE1_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NOISE2', 'MODEL_ANIME_STYLE_ART_RGB_NOISE2_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NOISE3', 'MODEL_ANIME_STYLE_ART_RGB_NOISE3_TTA', 'MODEL_ANIME_STYLE_ART_RGB_NO_NOISE', 'MODEL_ANIME_STYLE_ART_RGB_NO_NOISE_TTA', 'MODEL_CUNET_NOISE0', 'MODEL_CUNET_NOISE0_TTA', 'MODEL_CUNET_NOISE1', 'MODEL_CUNET_NOISE1_TTA', 'MODEL_CUNET_NOISE2', 'MODEL_CUNET_NOISE2_TTA', 'MODEL_CUNET_NOISE3', 'MODEL_CUNET_NOISE3_TTA', 'MODEL_CUNET_NO_NOISE', 'MODEL_CUNET_NO_NOISE_TTA', 'MODEL_CUNET_NO_SCALE_NOISE0', 'MODEL_CUNET_NO_SCALE_NOISE0_TTA', 'MODEL_CUNET_NO_SCALE_NOISE1', 'MODEL_CUNET_NO_SCALE_NOISE1_TTA', 'MODEL_CUNET_NO_SCALE_NOISE2', 'MODEL_CUNET_NO_SCALE_NOISE2_TTA', 'MODEL_CUNET_NO_SCALE_NOISE3', 'MODEL_CUNET_NO_SCALE_NOISE3_TTA', 'MODEL_CUNET_NO_SCALE_NO_NOISE', 'MODEL_CUNET_NO_SCALE_NO_NOISE_TTA', 'MODEL_PHOTO_NOISE0', 'MODEL_PHOTO_NOISE0_TTA', 'MODEL_PHOTO_NOISE1', 'MODEL_PHOTO_NOISE1_TTA', 'MODEL_PHOTO_NOISE2', 'MODEL_PHOTO_NOISE2_TTA', 'MODEL_PHOTO_NOISE3', 'MODEL_PHOTO_NOISE3_TTA', 'MODEL_PHOTO_NO_NOISE', 'MODEL_PHOTO_NO_NOISE_TTA'

# add picture ...
# waifu2x.add(data=imgData, modelIndex=waifu2x_vulkan.MODEL_ANIME_STYLE_ART_RGB_NOISE0, backId=0, scale=2.5)
# waifu2x.add(data=imgData, modelIndex=waifu2x_vulkan.MODEL_ANIME_STYLE_ART_RGB_NOISE0, backId=0, format="webp", width=1000, high=1000)

# load picture...
# newData, format, backId, tick = waifu2x.load(0)
```

"""
Version = "1.1.6"

Plat = sys.platform

print(Plat)

setuptools.setup(
    name="waifu2x-vulkan",
    version=Version,
    author="tonquer",
    license="MIT",
    author_email="tonquer@qq.com",
    description="A waifu2x python tool, use nihui/waifu2x-ncnn-vulkan",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/tonquer/waifu2x-vulkan",
    packages=setuptools.find_packages(),
    install_requires=[],
    classifiers=[
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        "License :: OSI Approved :: MIT License",
    ],
    entry_points={
        "pyinstaller40": [
            "hook-dirs = waifu2x_vulkan:get_hook_dirs"
        ]
    },
    python_requires = ">=3.6",
    include_package_data=True,
)
# python setup2.py bdist_wheel --plat-name=win-amd64 --python-tag=cp36.cp37.cp38.cp39.cp310