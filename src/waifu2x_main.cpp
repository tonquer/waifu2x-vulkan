#include "waifu2x_main.h"
#include "wic_image.h"
#include "webp_image.h"
#include <cmath>

TaskQueue Toproc;
TaskQueue Tosave;

int waifu2x_getData(void*& out, unsigned long& outSize, double& tick, int& callBack, unsigned int timeout = 10)
{
    Task v;

    Tosave.get(v, timeout);
    if (v.id == 0)
        return -1;

    if (v.id == -233)
        return -1;
    callBack = v.callBack;
    if (!v.isSuc) {
        return v.id;
    }
    out = v.out;
    outSize = v.outSize;

    v.out = NULL;
    const char* name;
    if (GpuId >= 0)
        name = ncnn::get_gpu_info(GpuId).device_name();
    else
        name = "cpu";
    //fprintf(stdout, "gpu%d:%s, end encode imageId :%d, h:%d, w:%d, encode:%f, proc:%f\n, decode:%f",
    //    GpuId, name, v.id, v.callBack, v.toH, v.toW,
    //    (double)(v.encodeTick - v.startTick) / CLOCKS_PER_SEC,
    //    (double)(v.procTick - v.encodeTick) / CLOCKS_PER_SEC,
    //    (double)(v.saveTick - v.procTick) / CLOCKS_PER_SEC);
    return v.id;
}

void* waifu2x_proc(void* args)
{
    const Waifu2x* waifu2x;
    for (;;)
    {
        Task v;

        Toproc.get(v);

        if (v.id == -233)
            break;
        if (v.modelIndex >= Waifu2xList.size())
        {
            v.isSuc = false; Tosave.put(v); continue;
        }
        else
        {
            waifu2x = Waifu2xList[v.modelIndex];
        }
        
        v.startTick = clock();
        unsigned char* pixeldata = 0;
        int w;
        int h;
        int c;

        pixeldata = wic_decode_image_by_data((unsigned char*)v.fileDate, v.fileSize, &w, &h, &c);
        if (v.fileDate) { free(v.fileDate); v.fileDate = NULL; }
        if (waifu2x && pixeldata)
        {
            v.inimage = ncnn::Mat(w, h, (void*)pixeldata, (size_t)c, c);
            if (v.toH <= 0 || v.toW <= 0)
            {
                v.toH = ceil(v.scale * h);
                v.toW = ceil(v.scale * w);
            }

            if (c == 4)
            {
                v.file = "png";
            }
            v.encodeTick = clock();
            //waifu2x->process(v.inimage, v.outimage);
            int scale_run_count = 1;
            if (v.toH > 0 && v.toW > 0 && h > 0 && w > 0)
            {
                scale_run_count = std::max(int((v.toW -1)/ w), scale_run_count);
                scale_run_count = std::max(int((v.toH -1)/ h), scale_run_count);
                scale_run_count = ceil(sqrt(scale_run_count));
            }

            v.outimage = ncnn::Mat(w * waifu2x->scale * scale_run_count, h * waifu2x->scale * scale_run_count, (size_t)c, c);

            for (int i = 0; i < scale_run_count; i++)
            {
                if (i == scale_run_count - 1)
                {
                    waifu2x->process(v.inimage, v.outimage);
                    v.inimage.release();
                }
                else
                {
                    ncnn::Mat tmpimage(v.inimage.w * 2, v.inimage.h * 2, (size_t)v.inimage.elemsize, (int)v.inimage.elemsize);
                    waifu2x->process(v.inimage, tmpimage);
                    v.inimage = tmpimage;
                }
            }
            v.procTick = clock();

            int success = 0;
            if (!v.file.compare("png") || !v.file.compare("PNG"))
            {
                success = wic_encode_image_to_data(v.outimage.w, v.outimage.h, v.outimage.elempack, v.outimage.data, v.out, v.outSize, v.modelIndex, v.toW, v.toH, w, h);
            }
            else if (!v.file.compare("jpg") || !v.file.compare("JPG") || !v.file.compare("jpeg") || !v.file.compare("JPEG"))
            {
                success = wic_encode_jpeg_image_to_data(v.outimage.w, v.outimage.h, v.outimage.elempack, v.outimage.data, v.out, v.outSize, v.modelIndex, v.toW, v.toH, w, h);
            }
            if (success)
            {
            }
            else
                fwprintf(stderr, L"encode image %ls failed\n", v.outpath.c_str());
            v.saveTick = clock();
        }
        else
        {
            fwprintf(stderr, L"decode image failed\n");
            v.isSuc = false;
        }
        Tosave.put(v);
    }

    return 0;
}

int waifu2x_addModel(const char* name, int scale2, int noise2, int tta_mode, int num_threads, const char * setModel)
{
    char parampath[256];
    char modelpath[256];
    Waifu2xList.push_back(NULL);
    int index = Waifu2xList.size() - 1;
    if (setModel && strcmp(name, "models-cunet"))
        return 0;

    if (noise2 == -1)
    {
        sprintf(parampath, "models/%s/scale2.0x_model.param", name);
        sprintf(modelpath, "models/%s/scale2.0x_model.bin", name);
    }
    else
    {
        sprintf(parampath, "models/%s/noise%d_scale2.0x_model.param", name, noise2);
        sprintf(modelpath, "models/%s/noise%d_scale2.0x_model.bin", name, noise2);
    }
    int prepadding = 18;
    int tilesize = 0;
    uint32_t heap_budget;
    if (GpuId == -1) heap_budget = 4000;
    else heap_budget = ncnn::get_gpu_device(GpuId)->get_heap_budget();

    if (!strcmp(name, "models-cunet"))
    {
        if (noise2 == -1)
        {
            prepadding = 18;
        }
        else if (scale2 == 1)
        {
            prepadding = 28;
        }
        else if (scale2 == 2)
        {
            prepadding = 18;
        }
        if (heap_budget > 2600)
            tilesize = 400;
        else if (heap_budget > 740)
            tilesize = 200;
        else if (heap_budget > 250)
            tilesize = 100;
        else
            tilesize = 32;
    }
    else if (!strcmp(name, "models-upconv_7_anime_style_art_rgb"))
    {
        prepadding = 7;
        if (heap_budget > 1900)
            tilesize = 400;
        else if (heap_budget > 550)
            tilesize = 200;
        else if (heap_budget > 190)
            tilesize = 100;
        else
            tilesize = 32;
    }
    else if (!strcmp(name, "models-upconv_7_photo"))
    {
        prepadding = 7;
        if (heap_budget > 1900)
            tilesize = 400;
        else if (heap_budget > 550)
            tilesize = 200;
        else if (heap_budget > 190)
            tilesize = 100;
        else
            tilesize = 32;
    }
    struct stat buffer;


    if (stat(parampath, &buffer) != 0)
    {
        fprintf(stderr, "not found path %s\n", parampath);
        return -1;
    }
    if (stat(modelpath, &buffer) != 0)
    {
        fprintf(stderr, "not found path %s\n", modelpath);
        return -1;
    }
    //fprintf(stdout, "init model path %s\n", modelpath);
    _bstr_t t1 = parampath;
    std::wstring paramfullpath((wchar_t*)t1);

    _bstr_t t2 = modelpath;
    std::wstring modelfullpath((wchar_t*)t2);
    Waifu2x* waifu = new Waifu2x(GpuId, tta_mode, num_threads);
    waifu->load(paramfullpath, modelfullpath);
    waifu->noise = noise2;
    waifu->scale = scale2;
    if (GpuId == -1)
    {
        // cpu only
        tilesize = 4000;
    }

    waifu->tilesize = tilesize;
    waifu->prepadding = prepadding;
    Waifu2xList[index] = waifu;
    return 1;
}

int waifu2x_init()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ncnn::create_gpu_instance();
    return 0;
}

int waifu2x_init_set(int gpuId2, int threadNum, const char *setModel)
{
    if (gpuId2 < -1 || gpuId2 >  2) { fwprintf(stderr, L"gpuId error\n"); return -1; };
    if (threadNum <= 0 || threadNum > 32) { fwprintf(stderr, L"threadNum error\n"); return -1; };

    int jobs_proc = threadNum;

    if (gpuId2 == 0) gpuId2 = ncnn::get_default_gpu_index();
    int cpu_count = std::max(1, ncnn::get_cpu_count());
    int gpu_count = ncnn::get_gpu_count();
    if (gpuId2 < -1 || gpuId2 >= gpu_count)
    {
        fprintf(stderr, "invalid gpu device\n");
        ncnn::destroy_gpu_instance();
        return -1;
    }
    if (gpuId2 == -1)
    {
        jobs_proc = std::min(jobs_proc, cpu_count);
        TotalJobsProc += 1;
    }
    else
    {
        int gpu_queue_count = ncnn::get_gpu_info(gpuId2).compute_queue_count();
        jobs_proc = std::min(jobs_proc, gpu_queue_count);
        TotalJobsProc += jobs_proc;
    }

    int num_threads = gpuId2 == -1 ? jobs_proc : 1;
    int index = 0;
    std::string models[3] = { "models-cunet", "models-upconv_7_anime_style_art_rgb", "models-upconv_7_photo" };
    for (int i = 0; i < 3; i++)
    {
        std::string name = models[i];
        for (int j = -1; j <= 3; j++)
        {
            if (waifu2x_addModel(name.c_str(), 2, j, true, num_threads, setModel) < 0) { return -1; };
            if (waifu2x_addModel(name.c_str(), 2, j, false, num_threads, setModel) < 0) { return -1; };
        }
    }

    // waifu2x proc
    ProcThreads.resize(TotalJobsProc);
    {
        int total_jobs_proc_id = 0;
        if (gpuId2 == -1)
        {
            ProcThreads[total_jobs_proc_id++] = new ncnn::Thread(waifu2x_proc);
        }
        else
        {
            for (int j = 0; j < jobs_proc; j++)
            {
                ProcThreads[total_jobs_proc_id++] = new ncnn::Thread(waifu2x_proc);
            }
        }
    }
    GpuId = gpuId2;
    //fprintf(stdout, "init success, threadNum:%d\n", TotalJobsProc);
    return 0;
}


int waifu2x_addData(const unsigned char* data, unsigned int size, int callBack, int modelIndex, const char* format, unsigned long toW, unsigned long toH, float scale)
{
    Task v;
    TaskId ++;
    v.id = TaskId;
    v.fileDate = (void*)data;
    v.fileSize = size;
    v.callBack = callBack;
    v.modelIndex = modelIndex;
    v.toH = toH;
    v.toW = toW;
    v.scale = scale;
    if ((toH <= 0 || toW <= 0) && scale <= 0)
        return -1;
    if (format) v.file = format;
    Toproc.put(v);
    return TaskId;
}

int waifu2x_stop()
{
    waifu2x_clear();
    {
        Task end;
        end.id = -233;

        for (int i = 0; i < TotalJobsProc; i++)
        {
            Toproc.put(end);
        }

        for (int i = 0; i < TotalJobsProc; i++)
        {
            ProcThreads[i]->join();
            delete ProcThreads[i];
        }

    }
    ncnn::destroy_gpu_instance();
    return 0;
}

int waifu2x_clear()
{
    Toproc.clear();
    Tosave.clear();
    return 0;
}

int waifu2x_remove(std::set<int> &taskIds)
{
    Toproc.remove(taskIds);
    Tosave.remove(taskIds);
    return 0;
}