import os, sys
current_path = os.path.abspath(__file__)
os.chdir(os.path.dirname(current_path))
from waifu2x_vulkan import waifu2x_vulkan as waifu2x
import time

if __name__ == "__main__":

    # 初始化ncnn
    # init ncnn
    sts = waifu2x.init()

    isCpuModel = False
    if sts < 0:
        # cpu model
        isCpuModel = True
    print("init, code:{}".format(str(sts)))    

    # 获得Gpu列表
    # get gpu list
    print(waifu2x.getGpuInfo())

    # 选择Gpu,设置线程数
    # select gpu, set thread num
    if isCpuModel:
        sts = waifu2x.initSet(-1, 1)
    else:
        sts = waifu2x.initSet(0, 1)
    print("init set, code:{}".format(str(sts)))

    # 开启打印
    # open debug log
    waifu2x.setDebug(True)

    f = open("0.jpg", "rb")
    data = f.read()
    f.close()
    backId = 1
    count = 0
    
    # 设置长宽缩放2.5倍
    # start convert, by setting scale
    if waifu2x.add(data, waifu2x.MODEL_CUNET_NOISE3, backId, scale=2.5) > 0:
        count += 1
    backId = 2

    # 固定长宽
    # CPU模式默认tileSize=800，如果内存小，请调低改值
    # start convert, by setting width and high
    # CPU Model default tileSize=800, if the memory is too small, Please turn down
    if waifu2x.add(data, waifu2x.MODEL_CUNET_NOISE3, backId, format="png", width=900, high=800, tileSize=200) > 0:
        count += 1
    
    saveName = {
        1 : "1.jpg",
        2 : "2.png"
    }

    while count > 0:
        time.sleep(1)

        # 阻塞获取，也可放入到到线程中
        # block
        info = waifu2x.load(0)
        if not info:
            continue 
        count -= 1
        newData, status, backId, tick = info
        f = open(saveName.get(backId), "wb+")
        f.write(newData)
        f.close
    
    # free ncnn, close thread
    waifu2x.stop()
