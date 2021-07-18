#include "waifu2x_main.h"

//#if _WIN32
// image decoder and encoder with wic
//#include "wic_image.h"
//#else // _WIN32
// image decoder and encoder with stb
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_STDIO
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

//#endif // _WIN32

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
    if (v.fileDate) {
        free(v.fileDate);
        v.fileDate = NULL;
    }
    if (!v.isSuc) {
        return v.id;
    }
    out = v.out;
    outSize = v.outSize;

    v.out = NULL;
    fprintf(stdout, "[waifu2x] end encode imageId :%d, encode:%f, proc:%f, decode:%f, \n",
        v.callBack, (double)(v.encodeTick - v.startTick) / CLOCKS_PER_SEC,
        (double)(v.procTick - v.encodeTick) / CLOCKS_PER_SEC,
        (double)(v.saveTick - v.procTick) / CLOCKS_PER_SEC);
    tick = (double)(v.saveTick - v.startTick) / CLOCKS_PER_SEC;
    return v.id;
}

void* waifu2x_proc(void* args)
{
    const Waifu2x* waifu2x;
    for (;;)
    {
        Task v;

        Toproc.get(v);
        fprintf(stdout, "proc %d", v.id);
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
//#if _WIN32
//        pixeldata = wic_decode_image_by_data((unsigned char*)v.fileDate, v.fileSize, &w, &h, &c);
//#else // _WIN32
        pixeldata = stbi_load_from_memory((unsigned char*)v.fileDate, v.fileSize, &w, &h, &c, 0);
        if (pixeldata)
        {
            // stb_image auto channel
            if (c == 1)
            {
                // grayscale -> rgb
                stbi_image_free(pixeldata);
                pixeldata = stbi_load_from_memory((unsigned char*)v.fileDate, v.fileSize, &w, &h, &c, 3);
                c = 3;
            }
            else if (c == 2)
            {
                // grayscale + alpha -> rgba
                stbi_image_free(pixeldata);
                pixeldata = stbi_load_from_memory((unsigned char*)v.fileDate, v.fileSize, &w, &h, &c, 4);
                c = 4;
            }
        }
//#endif // _WIN32
        if (v.fileDate) { 
            free(v.fileDate); 
            v.fileDate = NULL; 
        }

        const char* name;
        if (GpuId >= 0)
            name = ncnn::get_gpu_info(GpuId).device_name();
        else
            name = "cpu";

        fprintf(stdout, "[waifu2x] start encode imageId :%d, gpu:%s, format:%s, model:%s, noise:%d, scale:%d, tta:%d\n",
            v.callBack, name, v.file.c_str(), waifu2x->mode_name.c_str(), waifu2x->noise, waifu2x->scale, waifu2x->tta_mode);

        if (waifu2x && pixeldata)
        {
            v.inimage = ncnn::Mat(w, h, (void*)pixeldata, (size_t)c, c);
            if (v.toH <= 0 || v.toW <= 0)
            {
                v.toH = ceil(v.scale * h);
                v.toW = ceil(v.scale * w);
            }

            //if (c == 4)
            //{
            //    v.file = "png";
            //}
            v.encodeTick = clock();
            //waifu2x->process(v.inimage, v.outimage);
            int scale_run_count = 1;
            if (v.toH > 0 && v.toW > 0 && h > 0 && w > 0)
            {
                scale_run_count = std::max(int(v.toW/ w), scale_run_count);
                scale_run_count = std::max(int(v.toH/ h), scale_run_count);
                scale_run_count = ceil(log(scale_run_count)/log(2));
                scale_run_count = std::max(scale_run_count, 1);
            }

            int toW = w * pow(waifu2x->scale, scale_run_count);
            int toH = h * pow(waifu2x->scale, scale_run_count);
            v.outimage = ncnn::Mat(toW, toH, (size_t)c, c);

            for (int i = 0; i < scale_run_count; i++)
            {
                if (i == scale_run_count - 1)
                {
                    fprintf(stdout, "[waifu2x] start encode imageId :%d, count:%d, h:%d->%d, w:%d->%d \n",
                        v.callBack, i + 1, v.inimage.h, v.outimage.h, v.inimage.w, v.outimage.w);
                    waifu2x->process(v.inimage, v.outimage);
                    v.inimage.release();
                }
                else
                {
                    ncnn::Mat tmpimage(v.inimage.w * 2, v.inimage.h * 2, (size_t)v.inimage.elemsize, (int)v.inimage.elemsize);
                    fprintf(stdout, "[waifu2x] start encode imageId :%d, count:%d, h:%d->%d, w:%d->%d \n",
                        v.callBack, i + 1, v.inimage.h, tmpimage.h, v.inimage.w, tmpimage.w);
                    waifu2x->process(v.inimage, tmpimage);
                    v.inimage.release();
                    v.inimage = tmpimage;
                }
            }
//#if _WIN32
//            if (pixeldata) { free(pixeldata); pixeldata = NULL; };
//#else
            if (pixeldata) stbi_image_free(pixeldata);
//#endif
            v.procTick = clock();

            int success = 0;
            if (!v.file.compare("bmp") || !v.file.compare("BMP"))
            {
                auto* odata = (unsigned char*)malloc(v.toW * v.toH * v.outimage.elempack);
                stbir_resize((unsigned char*)v.outimage.data, v.outimage.w, v.outimage.h, 0, odata, v.toW, v.toH, 0, STBIR_TYPE_UINT8, v.outimage.elempack, STBIR_ALPHA_CHANNEL_NONE, 0,
                    STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                    STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                    STBIR_COLORSPACE_SRGB, nullptr
                );
                WriteData data(v.toH, v.toW, v.outimage.elempack);
                stbi_write_bmp_to_func((stbi_write_func*)write_jpg_to_mem, (void*)&data, v.toW, v.toH, v.outimage.elempack, odata);
                stbi_image_free(odata);
                if (data.writeSize > 0)
                {
                    success = true;
                    v.out = malloc(data.writeSize);
                    v.outSize = data.writeSize;
                    memcpy(v.out, data.data, v.outSize);
                }
                else {
                    success = false;
                }
            }
            else if (!v.file.compare("gif") || !v.file.compare("GIF"))
            {
                WriteData data(v.toH, v.toW, v.outimage.elempack);
                stbi_write_bmp_to_func((stbi_write_func*)write_jpg_to_mem, (void*)&data, v.toW, v.toH, v.outimage.elempack, (unsigned char*)v.outimage.data);
                if (data.writeSize > 0)
                {
                    success = true;
                    v.out = malloc(data.writeSize);
                    v.outSize = data.writeSize;
                    memcpy(v.out, data.data, v.outSize);
                }
                else {
                    success = false;
                }
            }
            else if (!v.file.compare("png") || !v.file.compare("PNG"))
            {
                /*#if _WIN32
                    success = wic_encode_image_to_data(v.outimage.w, v.outimage.h, v.outimage.elempack, v.outimage.data, v.out, v.outSize, v.modelIndex, v.toW, v.toH, w, h);
                #else*/
                {
                    auto *odata = (unsigned char *) malloc(v.toW * v.toH * v.outimage.elempack);
                    stbir_resize((unsigned char *)v.outimage.data, v.outimage.w, v.outimage.h, 0, odata, v.toW, v.toH, 0, STBIR_TYPE_UINT8, v.outimage.elempack, STBIR_ALPHA_CHANNEL_NONE, 0,
                                STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                                STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                                STBIR_COLORSPACE_SRGB, nullptr
                    ); 
                    v.out = stbi_write_png_to_mem(odata, 0, v.toW, v.toH, v.outimage.elempack, &v.outSize);
                    success = (v.out != nullptr);
                    stbi_image_free(odata);
                } 
                //#endif
            }
            else if (!v.file.compare("jpg") || !v.file.compare("JPG") || !v.file.compare("jpeg") || !v.file.compare("JPEG"))
            {
                //#if _WIN32
                //    success = wic_encode_jpeg_image_to_data(v.outimage.w, v.outimage.h, v.outimage.elempack, v.outimage.data, v.out, v.outSize, v.modelIndex, v.toW, v.toH, w, h);
                //#else
                {
                    auto *odata = (unsigned char *) malloc(v.toW * v.toH * v.outimage.elempack);
                    stbir_resize((unsigned char *)v.outimage.data, v.outimage.w, v.outimage.h, 0, odata, v.toW, v.toH, 0, STBIR_TYPE_UINT8, v.outimage.elempack, STBIR_ALPHA_CHANNEL_NONE, 0,
                                STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                                STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                                STBIR_COLORSPACE_SRGB, nullptr
                    );
                    WriteData data(v.toH, v.toW, v.outimage.elempack);
                    stbi_write_jpg_to_func((stbi_write_func *)write_jpg_to_mem, (void *)&data, v.toW, v.toH,  v.outimage.elempack, odata, 100);
                    stbi_image_free(odata);
                    if (data.writeSize > 0)
                    {
                        success = true;
                        v.out = malloc(data.writeSize);
                        v.outSize = data.writeSize;
                        memcpy(v.out, data.data, v.outSize);
                    }else{
                        success = false;
                    }
                    
                }
                //#endif
            }
            v.outimage.release();
            if (success)
            {
            }
            else
            {
                const char* error = stbi_failure_reason();
                fprintf(stderr, "[waifu2x] encode image %d failed, %s\n", v.id, error);
            }
            v.saveTick = clock();
        }
        else
        {
            const char* error = stbi_failure_reason();
            fprintf(stderr, "[waifu2x] decode image %d failed, %s\n", v.id, error);
            v.isSuc = false;
        }
        Tosave.put(v);
    }

    return 0;
}

int waifu2x_addModel(const char* name, int scale2, int noise2, int tta_mode, int num_threads, int index)
{
    char parampath[256];
    char modelpath[256];
    // Waifu2xList.push_back(NULL);
    if (scale2 == 2) {

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
    }
    else if (scale2 == 1) {
        if (noise2 == -1)
        {
            sprintf(parampath, "models/%s/noise0_model.param", name);
            sprintf(modelpath, "models/%s/noise0_model.bin", name);
        }
        else
        {
            sprintf(parampath, "models/%s/noise%d_model.param", name, noise2);
            sprintf(modelpath, "models/%s/noise%d_model.bin", name, noise2);
        }
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
        fprintf(stderr, "[waifu2x] not found path %s\n", parampath);
        return Waifu2xError::NotModel;
    }
    if (stat(modelpath, &buffer) != 0)
    {
        fprintf(stderr, "[waifu2x] not found path %s\n", modelpath);
        return Waifu2xError::NotModel;
    }
#if _WIN32
    _bstr_t t1 = parampath;
    std::wstring paramfullpath((wchar_t*)t1);

    _bstr_t t2 = modelpath;
    std::wstring modelfullpath((wchar_t*)t2);
#else
    std::string paramfullpath(parampath);
    std::string modelfullpath(modelpath);
#endif
    Waifu2x* waifu = new Waifu2x(GpuId, tta_mode, num_threads, name);
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
#if _WIN32
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

    return ncnn::create_gpu_instance();
}

int waifu2x_init_set(int gpuId2, int threadNum)
{
    if (gpuId2 < -1 || gpuId2 >  2) { 
        fprintf(stderr, "[waifu2x] gpuId error, gpuId2:%d, threadNum:%d \n", gpuId2, threadNum);
        return -1; 
    };
    if (threadNum <= 0 || threadNum > 32) { return -1; };

    int jobs_proc = threadNum;

    if (gpuId2 == 0) gpuId2 = ncnn::get_default_gpu_index();
    int cpu_count = std::max(1, ncnn::get_cpu_count());
    int gpu_count = ncnn::get_gpu_count();
    if (gpuId2 < -1 || gpuId2 >= gpu_count)
    {
        fprintf(stderr, "[waifu2x] invalid gpu device\n");
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
    
    NumThreads = gpuId2 == -1 ? jobs_proc : 1;
    int index = 0;
    std::string models[3] = { "models-cunet", "models-upconv_7_anime_style_art_rgb", "models-upconv_7_photo" };
    for (int i = 0; i < 3; i++)
    {
        std::string name = models[i];
        for (int j = -1; j <= 3; j++)
        {
            Waifu2xList.push_back(NULL);
            Waifu2xList.push_back(NULL);
            //if (waifu2x_addModel(name.c_str(), 2, j, true, NumThreads, setModel) < 0) { return -1; };
            //if (waifu2x_addModel(name.c_str(), 2, j, false, NumThreads, setModel) < 0) { return -1; };
        }
    }
    for (int i = -1; i <= 3; i++)
    {
        Waifu2xList.push_back(NULL);
        Waifu2xList.push_back(NULL);
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

int waifu2x_check_init_model(int initModel)
{
    if (initModel < 0 || initModel >= Waifu2xList.size())
    {
        return Waifu2xError::NotModel;
    }
    if (Waifu2xList[initModel])
    {
        return 1;
    }
    int index = 0;
    std::string models[3] = { "models-cunet", "models-upconv_7_anime_style_art_rgb", "models-upconv_7_photo" };
    for (int i = 0; i < 3; i++)
    {
        std::string name = models[i];
        for (int j = -1; j <= 3; j++)
        {
            if (index == initModel) {
                return waifu2x_addModel(name.c_str(), 2, j, false, NumThreads, index);
            }
            index += 1;
            if (index == initModel){
                return waifu2x_addModel(name.c_str(), 2, j, true, NumThreads, index);
            }
            index += 1;
        }
    }
    for (int j = -1; j <= 3; j++)
    {
        if (index == initModel) {
            return waifu2x_addModel("models-cunet", 1, j, false, NumThreads, index);
        }
        index ++ ;
        if (index == initModel) {
            return waifu2x_addModel("models-cunet", 1, j, true, NumThreads, index);
        }
        index++;
    }
    return 1;
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
    int sts = waifu2x_check_init_model(modelIndex);
    if (sts < 0)
        return sts;

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

int waifu2x_py_remove_wait(std::set<int>& taskIds)
{
    Toproc.remove(taskIds);
    return 0;
}

int waifu2x_remove(std::set<int> &taskIds)
{
    Toproc.remove(taskIds);
    Tosave.remove(taskIds);
    return 0;
}