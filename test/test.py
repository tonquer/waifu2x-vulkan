import os, sys
sys.path.append("..")
curPath = os.getcwd()
import waifu2x_vulkan as waifu2x
import time

if __name__ == "__main__":

    # 为了方便加载models目录，所有切换为上级目录
    os.chdir(os.path.dirname(curPath))
    
    # 初始化ncnn
    waifu2x.init()

    # 获得Gpu列表
    print(waifu2x.getGpuInfo())

    # 选择Gpu,设置线程数
    waifu2x.initSet(0, 1)

    # 开启打印
    waifu2x.setDebug(True)

    f = open("test/0.jpg", "rb")
    data = f.read()
    f.close()
    backId = 1
    count = 0
    
    # 设置长宽缩放2.5倍
    if waifu2x.add(data, waifu2x.MODEL_CUNET_NOISE3, backId, scale=2.5) > 0:
        count += 1
    backId = 2

    # 固定长宽
    if waifu2x.add(data, waifu2x.MODEL_CUNET_NOISE3, backId, format="png", width=900, high=800) > 0:
        count += 1
    
    saveName = {
        1 : "test/1.jpg",
        2 : "test/2.png"
    }

    while count > 0:
        time.sleep(1)

        # 阻塞获取，也可放入到到线程中
        info = waifu2x.load(0)
        if not info:
            continue 
        count -= 1
        newData, status, backId, tick = info
        f = open(saveName.get(backId), "wb+")
        f.write(newData)
        f.close
    waifu2x.stop()
