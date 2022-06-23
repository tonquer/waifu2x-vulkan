#ifndef OTHER_IMAGE_H
#define OTHER_IMAGE_H

//#if _WIN32
// image decoder and encoder with wic
//#include "wic_image.h"
//#else // _WIN32
// image decoder and encoder with stb
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PNM
#define STBI_NO_PIC
#define STBI_NO_STDIO
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "waifu2x_main.h"
#include "webp_image.h"

bool bmp_save(Task &v)
{
    bool success;
    ncnn::Mat& outimage = **v.outImage.begin();
    unsigned char* odata = (unsigned char*)malloc(v.toW * v.toH * outimage.elempack);
    stbir_resize((unsigned char*)outimage.data, outimage.w, outimage.h, 0, odata, v.toW, v.toH, 0, STBIR_TYPE_UINT8, outimage.elempack, STBIR_ALPHA_CHANNEL_NONE, 0,
        STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
        STBIR_FILTER_BOX, STBIR_FILTER_BOX,
        STBIR_COLORSPACE_SRGB, nullptr
    );
    WriteData data(v.toH, v.toW, outimage.elempack);
    stbi_write_bmp_to_func((stbi_write_func*)write_jpg_to_mem, (void*)&data, v.toW, v.toH, outimage.elempack, odata);
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
    return success;
}

bool png_save(Task& v)
{
    bool success;
    /*#if _WIN32
    success = wic_encode_image_to_data(v.outimage.w, v.outimage.h, v.outimage.elempack, v.outimage.data, v.out, v.outSize, v.modelIndex, v.toW, v.toH, w, h);
#else*/
    {
        ncnn::Mat& outimage = **v.outImage.begin();
        unsigned char* odata = (unsigned char*)malloc(v.toW * v.toH * outimage.elempack);
        stbir_resize((unsigned char*)outimage.data, outimage.w, outimage.h, 0, odata, v.toW, v.toH, 0, STBIR_TYPE_UINT8, outimage.elempack, STBIR_ALPHA_CHANNEL_NONE, 0,
            STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
            STBIR_FILTER_BOX, STBIR_FILTER_BOX,
            STBIR_COLORSPACE_SRGB, nullptr
        );
        v.out = stbi_write_png_to_mem(odata, 0, v.toW, v.toH, outimage.elempack, &v.outSize);
        success = (v.out != nullptr);
        stbi_image_free(odata);
    }
    return success;
    //#endif
}

bool jpg_save(Task& v)
{
    bool success;
    ncnn::Mat& outimage = **v.outImage.begin();
    unsigned char* odata = (unsigned char*)malloc(v.toW * v.toH * outimage.elempack);
    stbir_resize((unsigned char*)outimage.data, outimage.w, outimage.h, 0, odata, v.toW, v.toH, 0, STBIR_TYPE_UINT8, outimage.elempack, STBIR_ALPHA_CHANNEL_NONE, 0,
        STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
        STBIR_FILTER_BOX, STBIR_FILTER_BOX,
        STBIR_COLORSPACE_SRGB, nullptr
    );
    WriteData data(v.toH, v.toW, outimage.elempack);
    stbi_write_jpg_to_func((stbi_write_func*)write_jpg_to_mem, (void*)&data, v.toW, v.toH, outimage.elempack, odata, 100);
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
    return success;
}


bool to_save(Task &v)
{
    if (!v.file.compare("bmp"))
    {
        return bmp_save(v);
    }
    else if (!v.file.compare("png"))
    {
        return png_save(v);
    }
    else if (!v.file.compare("jpg") || !v.file.compare("jpeg"))
    {
        return jpg_save(v);
    }
    else if (!v.file.compare("webp") || !v.file.compare("gif"))
    {
        if (v.outImage.size() <= 1)
            return webp_save(v);
        else
            return webp_save_ani(v);
    }
    return false;
}

bool stbi_xload(Task &v)
{
    bool ok = true;
    void* result = 0;
    int x = 0, y = 0, c = 0;
    int frames;
    int req_comp = 0;
    std::string format;
    stbi__context s;
    stbi__start_mem(&s, (unsigned char *)v.fileDate , v.fileSize);
    int* delays=0;
    void * pixeldata = 0;
    if (stbi__gif_test(&s))
    {
        pixeldata = stbi__load_gif_main(&s, &delays, &x, &y, &frames, &c, 4);
        if (!pixeldata)
        {
            ok = false;
            goto End;
        }
        format = "gif";
        for (int i = 0; i < frames; i++)
        {
            ncnn::Mat* inimage = new ncnn::Mat();
            inimage->create(x, y, (size_t)4, 4);
            int totalSize = x * y * 4;
            int blockSize = totalSize * i;
            memcpy(inimage->data, (unsigned char*)pixeldata+ blockSize, totalSize);
            int duration = delays[i];
            v.inImage.push_back(inimage);
            v.inFrame.push_back(duration);
        }
    }
    else {
        stbi__result_info ri;
        memset(&ri, 0, sizeof(ri)); // make sure it's initialized if we add new fields
        ri.bits_per_channel = 8; // default is 8 so most paths don't have to be changed
        ri.channel_order = STBI_ORDER_RGB; // all current input & output are this, but this is here so we can add BGR order
        ri.num_channels = 0;

        if (stbi__jpeg_test(&s)) { pixeldata = stbi__jpeg_load(&s, &x, &y, &c, req_comp, &ri); format = "jpg";}
        else if (stbi__png_test(&s))  {pixeldata = stbi__png_load(&s, &x, &y, &c, req_comp, &ri); format = "png";}
        else if (stbi__bmp_test(&s)) { pixeldata = stbi__bmp_load(&s, &x, &y, &c, req_comp, &ri);  format = "bmp";}
        if (!pixeldata) { ok = false; goto End; }
        if (ri.bits_per_channel != 8) {
            STBI_ASSERT(ri.bits_per_channel == 16);
            result = stbi__convert_16_to_8((stbi__uint16*)result, x, y, 4);
            ri.bits_per_channel = 8;
        }

        if (c == 1)
        {
            stbi_image_free(pixeldata);
            pixeldata = stbi_load_from_memory((unsigned char*)v.fileDate, v.fileSize, &x, &y, &c, 3);
            c = 3;
        }
        else if (c == 2)
        {
            // grayscale + alpha -> rgba
            stbi_image_free(pixeldata);
            pixeldata = stbi_load_from_memory((unsigned char*)v.fileDate, v.fileSize, &x, &y, &c, 4);
            c = 4;
        }

        if (!pixeldata) { ok = false; goto End; }
        ncnn::Mat *inimage = new ncnn::Mat();
        inimage->create(x, y, (size_t)c, c);
        memcpy(inimage->data, pixeldata, x * y * c);
        v.inImage.push_back(inimage);
    }
End:
    if (v.file.length() == 0) v.file = format;
    if (pixeldata) stbi_image_free(pixeldata);
    if (delays) stbi_image_free(delays);
    return ok;
}
//STBIDEF stbi_uc* stbi_xload(char const* filename, int** delays, int* width, int* height, int* frames, int* nrChannels)
//{
//    unsigned char* result;
//    stbi__context s;
//    result = stbi_load_gif_from_memory(filename, delays, width, height, frames, nrChannels, 0);
//
//    return result;
//}

bool to_load(Task& v)
{

    bool isSuc=true;
    isSuc = webp_load(v);
    if (!isSuc) 
    { 
        isSuc = webp_load_ani(v); 
    }
    if(!isSuc)
    {
        isSuc = stbi_xload(v);
    }
    return isSuc;
}
#endif