#include "waifu2x_main.h"
#include "other_image.h"

//#endif // _WIN32

#include <cmath>

TaskQueue Todecode;
TaskQueue Toproc;
TaskQueue Toencode;
TaskQueue Tosave;


static int GpuId;
static int TotalJobsProc = 0;
static int NumThreads = 1;
static int TaskId = 1;

bool IsDebug = false;

Waifu2xChar ModelPath[1024] = {0};

int waifu2x_getData(void*& out, unsigned long& outSize, double& tick, int& callBack, char * format, unsigned int timeout = 10)
{
    Task v;

    Tosave.get(v, timeout);
    if (v.id == 0)
    {
        waifu2x_set_error("waifu2x already stop");
        return -1;
    }

    if (v.id == -233)
    {
        waifu2x_set_error("waifu2x already stop");
        return -1;
    }
    callBack = v.callBack;
    if (v.fileDate) {
        free(v.fileDate);
        v.fileDate = NULL;
    }
    if (!v.isSuc) {
        waifu2x_set_error(v.err.c_str());
        return v.id;
    }
    out = v.out;
    outSize = v.outSize;

    v.out = NULL;
    strcpy(format, v.file.c_str());
    tick = v.allTick;
    return v.id;
}
void* waifu2x_decode(void* args)
{
    for (;;)
    {
        Task v;
        Todecode.get(v);
        if (v.id == -233)
            break;
        if (v.modelIndex >= (int)Waifu2xList.size())
        {
            v.isSuc = false; Tosave.put(v); continue;
        }
        ftime(&v.startTick);
        bool isSuc = to_load(v);
        ftime(&v.decodeTick);
        if (v.fileDate) {
            free(v.fileDate);
            v.fileDate = NULL;
        }
        if (isSuc)
        {
            Toproc.put(v);
        }
        else
        {
            v.isSuc = false;
            Tosave.put(v);
        }
    }
    return NULL;
}
void* waifu2x_encode(void* args)
{
    for (;;)
    {
        Task v;
        Toencode.get(v);
        if (v.id == -233)
            break;
        bool isSuc = to_save(v);
        v.clear_out_image();

        ftime(&v.encodeTick);
        double decodeTick = (v.decodeTick.time + v.decodeTick.millitm / 1000.0) - (v.startTick.time + v.startTick.millitm / 1000.0);
        double procTick = (v.procTick.time + v.procTick.millitm / 1000.0) - (v.decodeTick.time + v.decodeTick.millitm / 1000.0);
        double encodeTick = (v.encodeTick.time + v.encodeTick.millitm / 1000.0) - (v.procTick.time + v.procTick.millitm / 1000.0);
        double allTick = (v.encodeTick.time + v.encodeTick.millitm / 1000.0) - (v.startTick.time + v.startTick.millitm / 10000.0);
        v.allTick = allTick;
        waifu2x_printf(stdout, "[waifu2x] end encode imageId :%d, decode:%.2fs, proc:%.2fs, encode:%.2fs, \n",
            v.callBack, decodeTick, procTick, encodeTick);

        if (isSuc)
        {
            Tosave.put(v);
        }
        else
        {
            const char* err = stbi_failure_reason();
            if (err)
            {
                v.err = err;
            }
            v.isSuc = false; 
            Tosave.put(v);
        }

    }
    return NULL;
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

        waifu2x = Waifu2xList[v.modelIndex];

        const char* name;
        if (GpuId >= 0)
            name = ncnn::get_gpu_info(GpuId).device_name();
        else
            name = "cpu";

        waifu2x_printf(stdout, "[waifu2x] start encode imageId :%d, gpu:%s, format:%s, model:%s, noise:%d, scale:%d, tta:%d, tileSize:%d\n",
            v.callBack, name, v.file.c_str(), waifu2x->mode_name.c_str(), waifu2x->noise, waifu2x->scale, waifu2x->tta_mode, v.tileSize);
        int scale_run_count = 1;
        int frame = 0;
        for (std::list<ncnn::Mat *>::iterator in = v.inImage.begin(); in != v.inImage.end(); in++)
        {
            frame++;
            ncnn::Mat& inimage = **in;
            if (in == v.inImage.begin())
            {
                if (v.toH <= 0 || v.toW <= 0)
                {
                    v.toH = ceil(v.scale * inimage.h);
                    v.toW = ceil(v.scale * inimage.w);
                }

                //if (c == 4)
                //{
                //    v.file = "png";
                //}
                //waifu2x->process(v.inimage, v.outimage);
                scale_run_count = 1;
                if (v.toH > 0 && v.toW > 0 && inimage.h > 0 && inimage.w > 0)
                {
                    scale_run_count = std::max(int(ceil(v.toW * 1.0 / inimage.w)), scale_run_count);
                    scale_run_count = std::max(int(ceil(v.toH * 1.0 / inimage.h)), scale_run_count);
                    scale_run_count = ceil(log(scale_run_count) / log(2));
                    scale_run_count = std::max(scale_run_count, 1);
                }


                if (waifu2x->scale <= 1)
                {
                    scale_run_count = 1;
                }
            }
            int toW = inimage.w * pow(waifu2x->scale, scale_run_count);
            int toH = inimage.h * pow(waifu2x->scale, scale_run_count);
            ncnn::Mat* outimage = new ncnn::Mat(toW, toH, (size_t)inimage.elemsize, (int)inimage.elemsize);

            for (int i = 0; i < scale_run_count; i++)
            {
                if (i == scale_run_count - 1)
                {
                    waifu2x_printf(stdout, "[waifu2x] start encode imageId :%d, count:%d, frame:%d, h:%d->%d, w:%d->%d \n",
                        v.callBack, i + 1, frame, inimage.h, outimage->h, inimage.w, outimage->w);
                    waifu2x->process(inimage, *outimage, v.tileSize);
                    inimage.release();
                }
                else
                {
                    ncnn::Mat tmpimage(inimage.w * 2, inimage.h * 2, (size_t)inimage.elemsize, (int)inimage.elemsize);
                    waifu2x_printf(stdout, "[waifu2x] start encode imageId :%d, count:%d, frame:%d, h:%d->%d, w:%d->%d \n",
                        v.callBack, i + 1, frame, inimage.h, tmpimage.h, inimage.w, tmpimage.w);
                    waifu2x->process(inimage, tmpimage, v.tileSize);
                    inimage.release();
                    inimage = tmpimage;
                }
            }
            v.outImage.push_back(outimage);

        }
        ftime(&v.procTick);
        v.clear_in_image();
        Toencode.put(v);
    }
    return NULL;
}
void* waifu2x_to_stop(void* args)
{
    for (int i = 0; i < (int)OtherThreads.size(); i++)
    {
        OtherThreads[i]->join();
        delete OtherThreads[i];
    }
    for (int i = 0; i < TotalJobsProc; i++)
    {
        ProcThreads[i]->join();
        delete ProcThreads[i];
    }
    ncnn::destroy_gpu_instance();
    return 0;
}

int waifu2x_addModel(const Waifu2xChar* name, int scale2, int noise2, int tta_mode, int num_threads, int index)
{

    Waifu2xChar parampath[1024];
    Waifu2xChar modelpath[1024];
#if _WIN32
    // Waifu2xList.push_back(NULL);
    if (scale2 == 2) {

        if (noise2 == -1)
        {
            swprintf(parampath, L"%s/models/%s/scale2.0x_model.param", ModelPath, name);
            swprintf(modelpath, L"%s/models/%s/scale2.0x_model.bin", ModelPath, name);
        }
        else
        {
            swprintf(parampath, L"%s/models/%s/noise%d_scale2.0x_model.param", ModelPath, name, noise2);
            swprintf(modelpath, L"%s/models/%s/noise%d_scale2.0x_model.bin", ModelPath, name, noise2);
        }
    }
    else if (scale2 == 1) {
        if (noise2 == -1)
        {
            swprintf(parampath, L"%s/models/%s/noise0_model.param", ModelPath, name);
            swprintf(modelpath, L"%s/models/%s/noise0_model.bin", ModelPath, name);
        }
        else
        {
            swprintf(parampath, L"%s/models/%s/noise%d_model.param", ModelPath, name, noise2);
            swprintf(modelpath, L"%s/models/%s/noise%d_model.bin", ModelPath, name, noise2);
        }
    }
#else
    // Waifu2xList.push_back(NULL);
    if (scale2 == 2) {

        if (noise2 == -1)
        {
            sprintf(parampath, "%s/models/%s/scale2.0x_model.param", ModelPath, name);
            sprintf(modelpath, "%s/models/%s/scale2.0x_model.bin", ModelPath, name);
        }
        else
        {
            sprintf(parampath, "%s/models/%s/noise%d_scale2.0x_model.param", ModelPath, name, noise2);
            sprintf(modelpath, "%s/models/%s/noise%d_scale2.0x_model.bin", ModelPath, name, noise2);
        }
    }
    else if (scale2 == 1) {
        if (noise2 == -1)
        {
            sprintf(parampath, "%s/models/%s/noise0_model.param", ModelPath, name);
            sprintf(modelpath, "%s/models/%s/noise0_model.bin", ModelPath, name);
        }
        else
        {
            sprintf(parampath, "%s/models/%s/noise%d_model.param", ModelPath, name, noise2);
            sprintf(modelpath, "%s/models/%s/noise%d_model.bin", ModelPath, name, noise2);
        }
    }
#endif

    int prepadding = 18;
    int tilesize = 0;
    uint32_t heap_budget;
    if (GpuId == -1) heap_budget = 4000;
    else heap_budget = ncnn::get_gpu_device(GpuId)->get_heap_budget();
#if WIN32
    if (!wcscmp(name, L"models-cunet"))
#else
    if (!strcmp(name, "models-cunet"))
#endif
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

#if WIN32
    else if (!wcscmp(name, L"models-upconv_7_anime_style_art_rgb"))
#else
    else if (!strcmp(name, "models-upconv_7_anime_style_art_rgb"))
#endif
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
#if WIN32
    else if (!wcscmp(name, L"models-upconv_7_photo"))
#else
    else if (!strcmp(name, "models-upconv_7_photo"))
#endif
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
    if (GpuId == -1) tilesize = 400;

#if _WIN32

    struct _stat buffer;
    if (_wstat((wchar_t *)parampath, &buffer) != 0)
    {
        waifu2x_printf(stderr, L"[waifu2x] not found path %s\n", parampath);
        return Waifu2xError::NotModel;
    }
    if (_wstat((wchar_t *)modelpath, &buffer) != 0)
    {
        waifu2x_printf(stderr, L"[waifu2x] not found path %s\n", modelpath);
        return Waifu2xError::NotModel;
    }
#else

    struct stat buffer;
    if (stat((char *)parampath, &buffer) != 0)
    {
        waifu2x_printf(stderr, "[waifu2x] not found path %s\n", parampath);
        return Waifu2xError::NotModel;
    }
    if (stat((char *)modelpath, &buffer) != 0)
    {
        waifu2x_printf(stderr, "[waifu2x] not found path %s\n", modelpath);
        return Waifu2xError::NotModel;
    }
#endif

#if _WIN32
    //_bstr_t t1 = parampath;
    //std::wstring paramfullpath((wchar_t*)t1);

    //_bstr_t t2 = modelpath;
    //std::wstring modelfullpath((wchar_t*)t2);
    std::wstring paramfullpath(parampath);
    std::wstring modelfullpath(modelpath);

    _bstr_t b(name);
    const char* name2 = b;
#else
    std::string paramfullpath(parampath);
    std::string modelfullpath(modelpath);
    const char* name2 = name;
#endif
    Waifu2x* waifu = new Waifu2x(GpuId, tta_mode, num_threads, name2);
    waifu->load(paramfullpath, modelfullpath);
    waifu->noise = noise2;
    waifu->scale = scale2;
    //if (GpuId == -1)
    //{
        // cpu only
        //tilesize = 4000;
    //}

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

int waifu2x_get_path_size()
{
#if _WIN32
    return wcslen(ModelPath);
#else
    return strlen(ModelPath);
#endif
}

int waifu2x_init_path(const Waifu2xChar* modelPath2)
{
#if _WIN32
    if (modelPath2)
    {
        memset(ModelPath, 0, 1024);
        wcscpy(ModelPath, modelPath2);
    };
#else
    if (modelPath2)
    {
        memset(ModelPath, 0, 1024);
        strcpy(ModelPath, modelPath2);
    };
#endif
    return 0;
}

int waifu2x_init_set(int gpuId2, int cpuNum)
{
    if (cpuNum < 0 || cpuNum > 128) 
    { 
        waifu2x_set_error("invalid cpu num params");
        return -1; 
    };

    int jobs_proc = cpuNum;

    //if (gpuId2 == 0) gpuId2 = ncnn::get_default_gpu_index();
    int cpu_count = std::max(1, ncnn::get_cpu_count());
    int gpu_count = ncnn::get_gpu_count();
    if (gpu_count == 0) gpuId2 = -1;

    if (gpuId2 < -1 || gpuId2 >= gpu_count)
    {
        waifu2x_set_error("invalid gpu device index params");
        return -1;
    }
    if (gpuId2 == -1)
    {
        jobs_proc = std::min(jobs_proc, cpu_count);
        if (jobs_proc <= 0) { jobs_proc = std::max(1, cpu_count / 2); }
        NumThreads = jobs_proc;
        TotalJobsProc = 1;
    }
    else
    {
        int gpu_queue_count = ncnn::get_gpu_info(gpuId2).compute_queue_count();
        if (TotalJobsProc <= 0) { TotalJobsProc = std::min(2, gpu_queue_count); }
        NumThreads = 1;
    }
    
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

        ncnn::Thread *load_thread = new ncnn::Thread(waifu2x_encode);
        ncnn::Thread* save_thread = new ncnn::Thread(waifu2x_decode);
        OtherThreads.push_back(load_thread);
        OtherThreads.push_back(save_thread);

        int total_jobs_proc_id = 0;
        for (int j = 0; j < TotalJobsProc; j++)
        {
            ProcThreads[total_jobs_proc_id++] = new ncnn::Thread(waifu2x_proc);
        }
    }
    GpuId = gpuId2;
    //waifu2x_printf(stdout, "init success, threadNum:%d\n", TotalJobsProc);
    return 0;
}

int waifu2x_check_init_model(int initModel)
{
    if (initModel < 0 || initModel >= (int)Waifu2xList.size())
    {
        return Waifu2xError::NotModel;
    }
    if (Waifu2xList[initModel])
    {
        return 1;
    }
    int index = 0;
#if _WIN32
    std::wstring models[3] = { L"models-cunet", L"models-upconv_7_anime_style_art_rgb", L"models-upconv_7_photo" };
#else
    std::string models[3] = { "models-cunet", "models-upconv_7_anime_style_art_rgb", "models-upconv_7_photo" };
#endif
    for (int i = 0; i < 3; i++)
    {
#if _WIN32
        std::wstring name = models[i];
#else
        std::string name = models[i];
#endif
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
#if _WIN32
            return waifu2x_addModel(L"models-cunet", 1, j, false, NumThreads, index);
#else
            return waifu2x_addModel("models-cunet", 1, j, false, NumThreads, index);
#endif
        }
        index ++ ;
        if (index == initModel) {
#if _WIN32
            return waifu2x_addModel(L"models-cunet", 1, j, true, NumThreads, index);
#else
            return waifu2x_addModel("models-cunet", 1, j, true, NumThreads, index);
#endif
        }
        index++;
    }
    return 1;
}


int waifu2x_addData(const unsigned char* data, unsigned int size, int callBack, int modelIndex, const char* format, unsigned long toW, unsigned long toH, float scale, int tileSize)
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
    v.tileSize = tileSize;
    if ((toH <= 0 || toW <= 0) && scale <= 0)
    {
        waifu2x_set_error("invalid scale params");
        return -1;
    }
    int sts = waifu2x_check_init_model(modelIndex);
    if (sts < 0)
    {
        waifu2x_set_error("invalid model index");
        return sts;
    }
    if (format) v.file = format;
    
    transform(v.file.begin(), v.file.end(), v.file.begin(), ::tolower);
    if (v.file.length() == 0 || !v.file.compare("jpg") || !v.file.compare("jpeg") || !v.file.compare("png") || !v.file.compare("webp") || !v.file.compare("jpg") || !v.file.compare("bmp") || !v.file.compare("apng"))
    {
        Todecode.put(v);
        return TaskId;
    }

    waifu2x_set_error("invalid pictrue format");
    return -1;
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

        Task end2;
        end2.id = -233;
        Tosave.put(end2);
        Toencode.put(end2);
        Todecode.put(end2);
        ncnn::Thread t = ncnn::Thread(waifu2x_to_stop);
    }
    return 0;
}

int waifu2x_clear()
{   
    Toencode.clear();
    Toproc.clear();
    Todecode.clear();
    Tosave.clear();
    return 0;
}

int waifu2x_set_debug(bool isdebug)
{
    IsDebug = isdebug;
    return 0;
}

int waifu2x_remove_wait(std::set<int>& taskIds)
{
    Todecode.remove(taskIds);
    Toproc.remove(taskIds);
    return 0;
}

int waifu2x_remove(std::set<int> &taskIds)
{
    Toencode.remove(taskIds);
    Toproc.remove(taskIds);
    Todecode.remove(taskIds);
    Tosave.remove(taskIds);
    return 0;
}

int waifu2x_printf(void* p, const char* fmt, ...)
{
    if (IsDebug) {
        FILE* f = (FILE*)p;
        va_list vargs;
        int result;
        va_start(vargs, fmt);
        result = vfprintf(f, fmt, vargs);
        va_end(vargs);
        return result;
    }
        return 0;
}

int waifu2x_printf(void* p, const wchar_t* fmt, ...)
{
    if (IsDebug) {
        FILE* f = (FILE*)p;
        va_list vargs;
        int result;
        va_start(vargs, fmt);
        result = vfwprintf(f, fmt, vargs);
        va_end(vargs);
        return result;
    }
    return 0;
}

void waifu2x_set_error(const char* err)
{
    ErrMsg = err;
}

std::string waifu2x_get_error()
{
    return ErrMsg;
}