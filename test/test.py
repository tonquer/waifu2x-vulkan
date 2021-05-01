## 请把waifu2x.pyd和models目录放入到test目录下
import waifu2x
import time

if __name__ == "__main__":
    waifu2x.init()
    print(waifu2x.getGpuInfo())
    waifu2x.initSet(0, 1)
    f = open("0.jpg", "rb")
    data = f.read()
    f.close()
    backId = 1
    count = 0
    if waifu2x.add(data, waifu2x.MODEL_CUNET_NOISE3, backId, scale=2.5) > 0:
        count += 1
    backId = 2
    if waifu2x.add(data, waifu2x.MODEL_CUNET_NOISE3, backId, format="png", width=900, high=800) > 0:
        count += 1
    
    saveName = {
        1 : "1.jpg",
        2 : "2.png"
    }

    while count > 0:
        time.sleep(1)
        info = waifu2x.load(1)
        if not info:
            continue 
        count -= 1
        newData, status, backId, tick = info
        f = open(saveName.get(backId), "wb+")
        f.write(newData)
        f.close
    waifu2x.stop()
